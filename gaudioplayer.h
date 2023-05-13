#ifndef GAUDIOPLAYER_H
#define GAUDIOPLAYER_H

#include <QObject>
#include <QAudio>
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

private slots:
    void readBuffer();
    void decodingChanged (bool status);
    void audioSinkStateChanged(QAudio::State state);
    void sync();


signals:
    void loadStatus(qint64 position, bool isEnd);
    void positionChanged (qint64 position);

private:
    QMutex * timerMutex = nullptr;
    QBuffer  * buffer = nullptr;
    //QFile  * buffer = nullptr;
    QAudioDecoder * audioDecoder = nullptr;
    QAudioOutput * audioOut = nullptr;
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
};

#endif // GAUDIOPLAYER_H
