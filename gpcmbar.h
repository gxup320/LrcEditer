#ifndef GPCMBAR_H
#define GPCMBAR_H

#include <QLabel>
#include <QObject>
#include <QWidget>

class GLrc;
class QThread;

struct lrcPosItem
{
    QRect rect;
    qint64 time;
    int lrcId;
    QString string;
};

class GPcmbar : public QLabel
{
    Q_OBJECT
public:
    GPcmbar(QWidget *parent = nullptr);
    ~GPcmbar();
    void setPcm(QByteArray pcm);//同时设置长度
    void setTime(qint64 time);
    void setPos(qint64 pos);
    void setLrc(GLrc* lrc);
    void drawPcm();
public slots:
    void displayPcm();

signals:
    void valFromMouse(qint64 val);
    void drawPcmCached();

private:
    QPixmap * m_showImage;
    QPixmap * m_showImageNext = nullptr;
    QPixmap * m_pcmImage;
    QPixmap * m_lrcImage;
    QPixmap * m_lrcShowImage = nullptr;
    qint64 m_length = 0;
    qint64 m_pos = 0;
    qint64 m_displayPos = 0;
    qint64 m_moveing = -1;
    bool m_pause = false;
    qint64 m_pausePos = 0;
    GLrc * m_lrc = nullptr;
    int m_onLrcItem = -1;
    qint64 m_lrcPos = -1;
    QList<lrcPosItem> m_lrcPosItems;
    void formatLrc();
    void drowLrc();
    void resizeEvent(QResizeEvent *event);
    void enterEvent(QEnterEvent *event);
    void leaveEvent(QEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    QThread* pcmThread = nullptr;
    bool threadRunning;
    static void pcmDispaleThread(GPcmbar* pcmbar);
    bool displayed = true;
};

#endif // GPCMBAR_H
