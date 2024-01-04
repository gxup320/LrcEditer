#ifndef GPCMBARGL_H
#define GPCMBARGL_H

#include <QOpenGLWidget>
#include <QObject>
#include <QWidget>

class GLrc;

class GPcmbarGL : public QOpenGLWidget
{
    Q_OBJECT
    struct lrcPosItem
    {
        QRect rect;
        qint64 time;
        int lrcId;
        QString string;
    };
public:
    GPcmbarGL(QWidget *parent = nullptr);
    ~GPcmbarGL();
    void setPcm(QByteArray pcm);//同时设置长度
    void setTime(qint64 time);
    void setPos(qint64 pos);
    void setLrc(GLrc* lrc);
    void pause(bool p);

public slots:

signals:
    void valFromMouse(qint64 val);

private:
    QTimer* m_timer;
    QPixmap * m_pcmImage;
    qint64 m_length = 0;
    qint64 m_pos = 0;
    qint64 m_displayPos = 0;
    qint64 m_moveing = -1;
    bool m_pause = false;
    qint64 m_pausePos = 0;
    int m_onLrcItem = -1;
    qint64 m_lrcPos = -1;
    void paintEvent(QPaintEvent *e);
    GLrc * m_lrc = nullptr;
    QList<lrcPosItem> m_lrcPosItems;
    void formatLrc();
    void enterEvent(QEnterEvent *event);
    void leaveEvent(QEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
};

#endif // GPCMBARGL_H
