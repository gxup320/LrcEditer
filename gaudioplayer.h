#ifndef GAUDIOPLAYER_H
#define GAUDIOPLAYER_H

#include <QObject>
#include <QAudio>
#include <QProcess>
#include <QMediaMetaData>
class QBuffer;
class QAudioDecoder;
class QAudioOutput;
class QAudioFormat;
class QAudioSink;
class QElapsedTimer;
class QMutex;
class QFile;


class GAudioPlayer : public QObject
{
    Q_OBJECT
public:
    explicit GAudioPlayer(QObject *parent = nullptr);
    ~GAudioPlayer();
    void load(QUrl url);
    bool setPosition(qint64 position);
    qint64 position(bool trueProcess = false);
    void play(qint64 position = -1);
    void stop();
    bool isPlaying();
    void setVolume(float volume);
    void accurateProgress(bool isAccurate);
    qint64 setBufferSize(qint64 size = -1, qint64 smallSize = -1);
    qint64 getBufferSize();
    qint64 getBufferSizeSmall();

private slots:
    //void readBuffer();
    void audioSinkStateChanged(QAudio::State state);
    void sync();

    void stateChangedMateDate(QProcess::ProcessState newState);
    void stateChangedDecoder(QProcess::ProcessState newState);


signals:
    void loadStatus(qint64 position, bool isEnd);
    void positionChanged (qint64 position);
    void metaDataChanged (QMediaMetaData metaData);
    void durationChanged(qint64 length);
    void bufferSizeChanged(qint64 length);

private:
    QAudioFormat* format;
    qint64 bufferSize = 441000;
    qint64 bufferSizeSmall = 10000;
    QProcess * ffmpeg_mateDate;
    QProcess * ffmpeg_decoder;
    QString ffmpeg;
    QString ffmpeg_outJpg;
    QString ffmpeg_outPCM;
    QString ffmpeg_sourceDir;
    QMutex * timerMutex = nullptr;
    QBuffer  * buffer = nullptr;
    //QFile  * buffer = nullptr;
    //QAudioDecoder * audioDecoder = nullptr;
    //QAudioOutput * audioOut = nullptr;
    QAudioFormat * audioFormat = nullptr;
    QAudioSink * audioSink = nullptr;
    QByteArray * byteArry = nullptr;
    qint64 lastDecodeTime = -1;
    QElapsedTimer * playTime = nullptr;
    qint64 startTime = 0;
    bool playing = false;
    float lastVolume = 1;
    bool isAccurateProgress = true;
    qint64 musicLen = 0;
    qint64 getAudioFormatSize(QAudioFormat format);

    QThread * positionChangedThreadHandle;
    bool threadRunning;
    bool decodeing = false;
    friend void positionChangedThread(GAudioPlayer* audioPlayer);
    void decodingChanged (bool status);
    QString strMid(QString src, QString start, QString end);
};

#endif // GAUDIOPLAYER_H
