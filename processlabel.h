#ifndef PROCESSLABEL_H
#define PROCESSLABEL_H

#include <QLabel>
#include <QObject>
#include <QWidget>

class GLrc;

struct lrcPosItem
{
    QRect rect;
    qint64 time;
    int lrcId;
    QString string;
};

class ProcessLabel : public QLabel
{
    Q_OBJECT
public:
    ProcessLabel(QWidget *parent = nullptr);
    void setPcm(QByteArray pcm);//同时设置长度
    void setTime(qint64 time);
    void setPos(qint64 pos);
    void setLrc(GLrc* lrc);
    void drawPcm();

signals:
    void valFromMouse(qint64 val);

private:
    QPixmap * m_showImage;
    QPixmap * m_pcmImage;
    QPixmap * m_lrcImage;
    QPixmap * m_lrcShowImage;
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
};

#endif // PROCESSLABEL_H
