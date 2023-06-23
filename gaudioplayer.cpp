#include "gaudioplayer.h"
#include <QBuffer>
#include <QAudioDecoder>
#include <QAudioOutput>
#include <QAudioDevice>
#include <QAudioFormat>
#include <QAudioSink>
#include <QUrl>
#include <QThread>
#include <QTime>
#include <QElapsedTimer>
#include <QMutex>
#include <QMutexLocker>
#include <QFile>
#include <QCoreApplication>
#include <QDir>
#include <QImage>

void positionChangedThread(GAudioPlayer* audioPlayer);
GAudioPlayer::GAudioPlayer(QObject *parent)
    : QObject{parent}
{
    if(QSysInfo::WordSize == 32)
        ffmpeg = "";
    else
        ffmpeg = QCoreApplication::applicationDirPath() + "/ffmpeg";
    ffmpeg_outJpg = QDir::tempPath() + "gaudio_" + QString::number(QCoreApplication::applicationPid()) + ".jpg";
    ffmpeg_outWav = QDir::tempPath() + "gaudio_" + QString::number(QCoreApplication::applicationPid()) + ".wav";
    ffmpeg_mateDate = new QProcess;
    ffmpeg_decoder = new QProcess;
    connect(ffmpeg_mateDate, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(stateChangedMateDate(QProcess::ProcessState)));
    connect(ffmpeg_decoder, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(stateChangedDecoder(QProcess::ProcessState)));
    timerMutex = new QMutex;
    byteArry = new QByteArray;
    buffer = new QBuffer;
    buffer->setBuffer(byteArry);
    //buffer = new QFile("tempAudioPcm");
    buffer->open(QIODevice::ReadWrite);
    audioDecoder = new QAudioDecoder;
    connect(audioDecoder, SIGNAL(bufferReady()), this, SLOT(readBuffer()));
    connect(audioDecoder, SIGNAL(isDecodingChanged(bool)), this, SLOT(decodingChanged(bool)));
    threadRunning = true;
    positionChangedThreadHandle = QThread::create(positionChangedThread, this);
    positionChangedThreadHandle->start();
}

GAudioPlayer::~GAudioPlayer()
{
    threadRunning = false;
    for(int i = 0; i < 100 && positionChangedThreadHandle->isRunning(); i++) thread()->msleep(100);
    if(positionChangedThreadHandle->isRunning())
    {
        qDebug() << "listen stop time out,to terminate...";
        positionChangedThreadHandle->terminate();
    }
    while (positionChangedThreadHandle->isRunning()) { thread()->msleep(1);}
    delete positionChangedThreadHandle;
    if(audioSink != nullptr)
    {
        audioSink->stop();
        delete audioSink;
    }
    audioDecoder->stop();
    delete audioDecoder;
    //buffer->remove();
    delete buffer;
    delete byteArry;
    //playTime->stop();
    delete playTime;
    delete timerMutex;
    delete ffmpeg_decoder;
    delete ffmpeg_mateDate;
    QDir dir;
    dir.remove(ffmpeg_outWav);
    dir.remove(ffmpeg_outJpg);
}

void GAudioPlayer::load(QUrl url)
{
    stop();
    decodeing = true;
    //执行ffmpeg
    qDebug() << url.toLocalFile();
    QDir dir;
    dir.remove(ffmpeg_outWav);
    dir.remove(ffmpeg_outJpg);
    ffmpeg_mateDate->terminate();
    ffmpeg_mateDate->start(ffmpeg, {"-i", url.toLocalFile(), "-y", ffmpeg_outJpg});
    ffmpeg_decoder->terminate();
    ffmpeg_decoder->start(ffmpeg, {"-i", url.toLocalFile(), "-y", ffmpeg_outWav});
}

bool GAudioPlayer::setPosition(qint64 position)
{
    if(position < 0)
        position = 0;
    if(position > musicLen)
    {
        position = musicLen;
    }
    QAudioFormat format = audioDecoder->audioFormat();
    qint64 filePosition = getAudioFormatSize(format) * format.sampleRate() * position / 1000;
    if(getAudioFormatSize(format) != 0)
        filePosition -= filePosition % getAudioFormatSize(format);
    bool rev = false;
    if(playing)
    {
        if(isAccurateProgress)
        {
            stop();
            rev = buffer->seek(filePosition);
            startTime = position;
            QMutexLocker locker(timerMutex);
            //locker.relock();
            if(audioSink != nullptr)
            {
                playTime = new QElapsedTimer;
                audioSink->start(buffer);
                playTime->start();
            }
            locker.unlock();
        }
        else
        {
            if(audioSink != nullptr)
            {
                qsizetype buffSize = audioSink->bufferSize();
                QAudioFormat format = audioDecoder->audioFormat();
                //qDebug() << buffSize / (getAudioFormatSize(format) * format.sampleRate() / 1000.0);
                if(buffSize / (getAudioFormatSize(format) * format.sampleRate() / 1000.0) < 100)
                {
                    if(buffSize > 0)
                    {
                        startTime = position - audioSink->processedUSecs() / 1000 - buffSize / (getAudioFormatSize(format) * format.sampleRate() / 1000.0);
                    }
                    else
                    {
                        startTime = position - audioSink->processedUSecs() / 1000;
                    }
                    rev = buffer->seek(filePosition);
                }
                else
                {
                    //stop();
                    rev = buffer->seek(filePosition);
                    startTime = position;
                    audioSink->start(buffer);
                }
            }
        }
    }
    else
    {
        startTime = position;
        rev = buffer->seek(filePosition);
    }
    return rev;
}

qint64 GAudioPlayer::position(bool trueProcess)
{
    qint64 pos = 0;
    if(isAccurateProgress && trueProcess == false)
    {
        QMutexLocker locker(timerMutex);
        //locker.relock();
        if(playTime == nullptr)
        {
            pos = startTime;
        }
        else
        {
            pos = startTime + playTime->elapsed();
        }
        locker.unlock();
    }
    else
    {
        if(audioSink != nullptr)
        {
            qsizetype buffSize = audioSink->bufferSize();
            if(buffSize > 0)
            {
                QAudioFormat format = audioDecoder->audioFormat();
                pos = startTime + audioSink->processedUSecs() / 1000 - buffSize / (getAudioFormatSize(format) * format.sampleRate() / 1000.0);
            }
            else
            {
                pos = startTime + audioSink->processedUSecs() / 1000;
            }
        }
        else
        {
            QAudioFormat format = audioDecoder->audioFormat();
            pos = qint64(buffer->pos() / (getAudioFormatSize(format) * format.sampleRate() / 1000.0));
        }
    }
    return pos;
}

void GAudioPlayer::play(qint64 _position)
{
    if(_position >= 0)
        setPosition(_position);
    else
    {
        _position = position();
    }
    if(audioSink != nullptr && playing == false)
    {
        startTime = _position;
        if(isAccurateProgress)
        {
            audioSink->start(buffer);
            QMutexLocker locker(timerMutex);
            //locker.relock();
            if(playTime != nullptr)
                delete playTime;
            playTime = new QElapsedTimer;
            playTime->start();
            locker.unlock();
        }
        else
        {
            audioSink->start(buffer);
        }
    }
}

void GAudioPlayer::stop()
{
    if(audioSink != nullptr)
    {
        audioSink->stop();
    }
    if(isAccurateProgress)
    {
        QMutexLocker locker(timerMutex);
        //locker.relock();
        if(playTime != nullptr)
        {
            startTime = startTime + playTime->elapsed();
            delete playTime;
            playTime = nullptr;
        }
        locker.unlock();
    }
}

bool GAudioPlayer::isPlaying()
{
    return playing;
}

void GAudioPlayer::setVolume(float volume)
{
    if(audioSink != nullptr)
        audioSink->setVolume(volume);
    lastVolume = volume;
}

void GAudioPlayer::accurateProgress(bool isAccurate)
{
    qint64 pos = position();
    isAccurateProgress = isAccurate;
    setPosition(pos);
}

void GAudioPlayer::readBuffer()
{
    QAudioBuffer audio = audioDecoder->read();
    QAudioFormat format = audio.format();
    buffer->write(audio.data<char>(), audio.frameCount() * getAudioFormatSize(format));
    qint64 decodeTime = audioDecoder->position();
    //qDebug() << format;
    if(decodeTime - lastDecodeTime > 1000)
    {
        emit loadStatus(decodeTime, false);
        lastDecodeTime = decodeTime;
    }
}

void GAudioPlayer::decodingChanged(bool status)
{
    decodeing = status;
    musicLen = 0;
    if(status == false)
    {
        qint64 len = audioDecoder->position();
        //QAudioBuffer audio = audioDecoder->read();
        QAudioFormat format = audioDecoder->audioFormat();
        if(getAudioFormatSize(format) * 1000 / format.sampleRate() != 0)
        {
            len = buffer->size() / getAudioFormatSize(format) * 1000 / format.sampleRate();
        }
        musicLen = len;
        qDebug() << len;
        buffer->seek(0);
        QMutexLocker locker(timerMutex);
        //locker.relock();
        if(playTime != nullptr)
        {
            delete playTime;
            playTime = nullptr;
        }
        startTime = 0;
        lastDecodeTime = -1;
        locker.unlock();
        audioDecoder->setSource(QUrl());
        emit loadStatus(len, true);
    }

}

void GAudioPlayer::audioSinkStateChanged(QAudio::State state)
{
    if(state == QAudio::ActiveState)
    {
        playing = true;
    }
    else
    {
        playing = false;
        QMutexLocker locker(timerMutex);
        //locker.relock();
        if(playTime != nullptr)
        {
            startTime = startTime + playTime->elapsed();
            delete playTime;
            playTime = nullptr;
        }
        locker.unlock();
    }
}

void GAudioPlayer::sync()
{

}

void GAudioPlayer::stateChangedMateDate(QProcess::ProcessState newState)
{
    QMediaMetaData metaData;
    if(newState == QProcess::NotRunning)
    {
        if(ffmpeg_mateDate->exitCode() == 0)
        {
            QString text = ffmpeg_mateDate->readAllStandardOutput() + ffmpeg_mateDate->readAllStandardError();
            qDebug() << strMid(text, "ARTIST          : ", "\r\n");
            metaData.insert(QMediaMetaData::AlbumArtist, strMid(text, "ARTIST          : ", "\r\n"));
            qDebug() << strMid(text, "TITLE           : ", "\r\n");
            metaData.insert(QMediaMetaData::Title, strMid(text, "TITLE           : ", "\r\n"));
            qDebug() << strMid(text, "ALBUM           : ", "\r\n");
            metaData.insert(QMediaMetaData::AlbumTitle, strMid(text, "ALBUM           : ", "\r\n"));
            qDebug() << strMid(text, "album_artist    : ", "\r\n");
            metaData.insert(QMediaMetaData::AlbumArtist, strMid(text, "album_artist    : ", "\r\n"));
            qDebug() << strMid(text, "GENRE           : ", "\r\n");
            metaData.insert(QMediaMetaData::Genre, strMid(text, "GENRE           : ", "\r\n"));
            qDebug() << strMid(text, "GENRENUMBER     : ", "\r\n");
            qDebug() << strMid(text, "COMPOSER        : ", "\r\n");
            metaData.insert(QMediaMetaData::Composer, strMid(text, "COMPOSER        : ", "\r\n"));
            qDebug() << strMid(text, "disc            : ", "\r\n");
            qDebug() << strMid(text, "track           : ", "\r\n");
            qDebug() << strMid(text, "COPYRIGHT       : ", "\r\n");
            metaData.insert(QMediaMetaData::Copyright, strMid(text, "COPYRIGHT       : ", "\r\n"));
            qDebug() << strMid(text, "ORGANIZATION    : ", "\r\n");
            metaData.insert(QMediaMetaData::Orientation, strMid(text, "ORGANIZATION    : ", "\r\n"));
            qDebug() << strMid(text, "COMMENT         : ", "\r\n");
            metaData.insert(QMediaMetaData::Comment, strMid(text, "COMMENT         : ", "\r\n"));
            qDebug() << strMid(text, "PERFORMER       : ", "\r\n");
            qDebug() << strMid(text, "MOOD            : ", "\r\n");
            QImage img;
            if(img.load(ffmpeg_outJpg))
            {
                metaData.insert(QMediaMetaData::ThumbnailImage, img);
            }
            QString timeStr = strMid(text, "Duration: ", ",");
            QTime time = QTime::fromString(timeStr + "0","hh:mm:ss.zzz");
            if(!time.isNull())
                emit durationChanged(time.msecsSinceStartOfDay());
            emit metaDataChanged(metaData);
        }
    }
}

void GAudioPlayer::stateChangedDecoder(QProcess::ProcessState newState)
{
    if(newState == QProcess::NotRunning)
    {
        if(ffmpeg_mateDate->exitCode() == 0)
        {
            QAudioFormat format;
            format.setChannelCount(2);
            format.setSampleFormat(QAudioFormat::Int16);
            format.setSampleRate(44100);
            audioDecoder->stop();
            audioDecoder->setAudioFormat(format);
            audioDecoder->setSource(ffmpeg_outWav);
            if(audioSink != nullptr)
            {
                delete audioSink;
            }
            audioSink = new QAudioSink(format, this);
            audioSink->setVolume(lastVolume);
            connect(audioSink, SIGNAL(stateChanged(QAudio::State)), this, SLOT(audioSinkStateChanged(QAudio::State)));
            //audioSink->setBufferSize(1);
            buffer->close();
            buffer->setData(QByteArray());
            //buffer->resize(0);
            buffer->open(QIODevice::ReadWrite);
            audioDecoder->start();
        }
    }
}


qint64 GAudioPlayer::getAudioFormatSize(QAudioFormat format)
{

    quint32 size = 0;
    if(format.sampleFormat() == QAudioFormat::UInt8)
    {
        size = 1;
    }
    else if(format.sampleFormat() == QAudioFormat::Int16)
    {
        size = 2;
    }
    else if(format.sampleFormat() == QAudioFormat::Int32)
    {
        size = 4;
    }
    else if(format.sampleFormat() == QAudioFormat::Float)
    {
        size = 4;
    }
    return size * format.channelCount();
}

QString GAudioPlayer::strMid(QString src, QString start, QString end)
{
    int startIndex = src.toUpper().indexOf(start.toUpper(), 0);
    if(startIndex == -1)
    {
        return "";
    }
    int endIndex = src.toUpper().indexOf(end.toUpper(), startIndex);
    if(endIndex == -1)
    {
        return "";
    }
    return src.mid(startIndex + start.length(), endIndex - startIndex - start.length());
}

void positionChangedThread(GAudioPlayer* audioPlayer)
{
    qint64 lastPos = -1;
    qint64 pos;
    while(audioPlayer->threadRunning)
    {
        if(audioPlayer->decodeing)
        {
            lastPos = -1;
        }
        else
        {
            pos = audioPlayer->position();
            if(pos != lastPos)
            {
                lastPos = pos;
                emit audioPlayer->positionChanged(pos);
                if(abs(audioPlayer->position(true) - pos) > 1000)
                {
                    audioPlayer->sync();
                }
                //if(audioPlayer->audioSink != nullptr)
                //    qDebug() << audioPlayer->audioSink->bufferSize();
            }
        }
        QObject().thread()->usleep(10);
    }
}
