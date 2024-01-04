#ifndef GLRCWINDOWGL_H
#define GLRCWINDOWGL_H

#include <QOpenGLWidget>
#include <QObject>
#include <QWidget>

class GLrc;

class GLrcWindowGL : public QOpenGLWidget
{
    Q_OBJECT
public:
    GLrcWindowGL(QWidget *parent = nullptr);
    ~GLrcWindowGL();
    void setLrc(GLrc* lrc);

    void setDispaleColor(const QColor& _default,const QColor& _selectLine,const QColor& _selectLineOver,const QColor& _selectWord);
    void setBackground(QColor color);
    void setBackground(QImage image);
    void copyTo(GLrcWindowGL* target);
    void pause(bool p);

public slots:
    void setTime(qint64 time);
    void discopy();

signals:
    void closeLrc();

private:
    GLrcWindowGL* copyed = nullptr;
    QTimer* m_timer = nullptr;
    qint64 m_time = 0;
    GLrc* m_lrc = nullptr;
    bool m_disableMovingPicture = true;
    int m_lrcY = 0;
    int m_speed = 0;
    int movSpeed(int length);
    void paintEvent(QPaintEvent *e);
    int m_dark = 0;
    QColor* colors;
    QColor* backgroundColor;
    QImage* backgroundImage;
    QPixmap* backgroundPixmap;
    //0-背景颜色，1-背景图标
    int backgroundMode = 0;
    void backgroundImageToPixmap();
    void resizeEvent(QResizeEvent *e);
    void closeEvent(QCloseEvent *event);
};

#endif // GLRCWINDOWGL_H
