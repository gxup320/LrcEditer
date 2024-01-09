#include "gpcmbargl.h"
#include <QBuffer>
#include <QPainter>
#include <QPixmap>
#include <QMouseEvent>
#include <QThread>
#include <QTimer>
#include "glrc.h"

GPcmbarGL::GPcmbarGL(QWidget *parent)
    :QOpenGLWidget(parent)
{
    setMouseTracking(true);
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(update()));
    m_timer->start(0);
}

GPcmbarGL::~GPcmbarGL()
{
    m_timer->stop();
    disconnect(m_timer, SIGNAL(timeout()), this, SLOT(update()));
    delete m_timer;
}

void GPcmbarGL::setPcm(QByteArray pcm)
{
    m_length = pcm.length() / 2 / 441;
    m_showBytearray = {};
    int seek = 0;
    while (seek < pcm.size() - 441 * 4)
    {
        //计算PCM
        float yMax = 0;
        float yMin = 0;
        for (int var = 0; var < 441; ++var)
        {
            short y = (short(pcm[seek + 1]) << 8) | pcm[seek + 0];
            if(yMax < y)
                yMax = y;
            if(yMin > y)
                yMin = y;
            seek+=2;
        }
        m_showBytearray.append(yMax / 0x7FFF);
        m_showBytearray.append(yMin / 0x7FFF);
    }
}

void GPcmbarGL::setTime(qint64 time)
{
    //将时间转换为偏移像素
    setPos(time / 5);
}

void GPcmbarGL::setPos(qint64 pos)
{
    if(pos != m_pos)
    {
        m_pos = pos;
    }
}

void GPcmbarGL::setLrc(GLrc *lrc)
{
    m_lrc = lrc;
}

void GPcmbarGL::pause(bool p)
{
    if(p)
    {
        m_timer->stop();
    }
    else
    {
        m_timer->start(0);
    }
}

void GPcmbarGL::formatLrc()
{
    if(m_lrc == nullptr)
    {
        return;
    }
    //清空lrc记录
    m_lrcPosItems.clear();
    QRect l_rect;
    GLrc l_lrc = *m_lrc;
    l_lrc.mergeDuplicates(GLrc::SPLIT);
    //pos to time
    qint64 time = (qint64)(m_displayPos - width() / 2) * 5;
    l_lrc.setSeleteTime(time);
    int seleteLine = l_lrc.getSelectLine();//从这一行开始画
    qint64 lrcTime = l_lrc.getLrcTime(time);
    qint64 nextTime = l_lrc.getLrcNextTime(lrcTime + 1);
    QString lrcLine = "";
    if(seleteLine == -1)
    {
        lrcTime = 0;
    }
    else
    {
        lrcLine = l_lrc.getLineString(seleteLine, false);
    }
    //第一行不完整，单独画出
    //获取第一行范围
    l_rect.setRect(0,0,(nextTime - lrcTime - (time - lrcTime)) / 5,200);
    while(l_rect.left() < width())
    {
        //将lrc信息加入数组
        lrcPosItem l_lrcPosItem;
        l_lrcPosItem.lrcId = seleteLine;
        l_lrcPosItem.rect = l_rect;
        l_lrcPosItem.time = lrcTime;
        l_lrcPosItem.string = lrcLine;
        m_lrcPosItems << l_lrcPosItem;
        //计算下一张
        l_rect.setLeft(l_rect.right());
        seleteLine++;
        if(seleteLine >= l_lrc.size())
        {
            break;
        }
        QList<qint64> time = l_lrc.getLineTime(seleteLine);
        if(time.length() < 1)
        {
            continue;
        }
        lrcTime = time[0];
        nextTime = l_lrc.getLrcNextTime(lrcTime + 1);
        l_rect.setRight(l_rect.left() + (nextTime - lrcTime) / 5);
        lrcLine = l_lrc.getLineString(seleteLine, false);
    }
}

void GPcmbarGL::enterEvent(QEnterEvent *event)
{
    m_pause = true;
    m_pausePos = m_displayPos;
}

void GPcmbarGL::leaveEvent(QEvent *event)
{
    m_pause = false;
    m_onLrcItem = -1;
}

void GPcmbarGL::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_moveing = event->pos().x();
        if(m_onLrcItem != -1)
        {
            m_lrcPos = m_lrcPosItems[m_onLrcItem].rect.x();
        }
    }
}

void GPcmbarGL::mouseMoveEvent(QMouseEvent *event)
{
    if(m_moveing != -1 && m_onLrcItem == -1)
    {
        //移动进度条
        m_displayPos = m_pausePos + m_moveing - event->pos().x();
        if(m_displayPos > m_length)
        {
            m_displayPos = m_length;
        }
        if(m_displayPos < 0)
        {
            m_displayPos = 0;
        }
    }
    else if(m_moveing != -1 && m_onLrcItem != -1)
    {
        //调整歌词
        int min = 0,max = width();
        if(m_onLrcItem > 0)
        {
            min = m_lrcPosItems[m_onLrcItem-1].rect.left() + 1;
        }
        if(min < 0)
        {
            min = 0;
        }
        if(min < width() / 2 - m_displayPos)
        {
            min = width() / 2 - m_displayPos;
        }
        if(m_onLrcItem < m_lrcPosItems.length() - 1)
        {
            max = m_lrcPosItems[m_onLrcItem+1].rect.left() - 1;
        }
        if(min > width())
        {
            min = width();
        }
        int posx = event->pos().x();
        if(posx > max)
        {
            posx = max;
        }
        if(posx < min)
        {
            posx = min;
        }
        m_lrcPosItems[m_onLrcItem].rect.setLeft(posx);
        if(m_onLrcItem > 0)
        {
            m_lrcPosItems[m_onLrcItem - 1].rect.setRight(posx);
        }
    }
    else
    {
        //判断鼠标是否落在歌词分割线上
        m_onLrcItem = -1;
        if(event->pos().x() > 2)
        {
            for (int var = 0; var < m_lrcPosItems.length(); ++var)
            {
                if(m_lrcPosItems[var].rect.x() + 2 > event->pos().x() && m_lrcPosItems[var].rect.x() - 3 < event->pos().x())
                {
                    //判定在歌词上
                    setCursor(Qt::SizeHorCursor);
                    m_onLrcItem = var;
                    break;
                }
            }
        }
        if(m_onLrcItem == -1)
        {
            setCursor(Qt::ArrowCursor);
        }
    }
}

void GPcmbarGL::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton && m_moveing != -1 && m_displayPos != m_pausePos && m_onLrcItem == -1)
    {
        //释放鼠标，发出移动消息
        m_pos = m_displayPos;
        m_pausePos = m_displayPos;
        emit valFromMouse(m_displayPos * 5);
    }
    if(event->button() == Qt::LeftButton && m_onLrcItem != -1 && m_moveing != -1 && m_lrcPos != m_lrcPosItems[m_onLrcItem].rect.x())
    {
        //调整歌词
        if(m_lrc != nullptr)
        {
            qint64 time = m_lrcPosItems[m_onLrcItem].time - (m_lrcPos - m_lrcPosItems[m_onLrcItem].rect.x()) * 5;
            m_lrc->replaceTime(m_lrcPosItems[m_onLrcItem].time, time);
            m_lrcPosItems[m_onLrcItem].time = time;
        }
    }
    m_moveing = -1;
}

void GPcmbarGL::initializeGL()
{
    initializeOpenGLFunctions();
}

void GPcmbarGL::paintGL()
{
    if(m_pause == false)
        m_displayPos = m_pos;
    if(m_onLrcItem == -1)
        formatLrc();
    glClearColor(0.0f,0.0f,0.0f,0.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor3f(0.0f,1.0f,0.0f);
    glBegin(GL_LINES);
    int seek = m_displayPos * 2 - width();
    int x = 0;
    int w = width();
    //先画一条中线
    glVertex2f(-1.0,0);
    glVertex2f(1.0,0);
    while (x < w && seek < m_showBytearray.size() - 2)
    {
        if(seek >= 0)
        {
            glVertex2f((x-w/2.0f)/(w/2.0f),m_showBytearray[seek]);
            glVertex2f((x-w/2.0f)/(w/2.0f),m_showBytearray[seek+1]);
        }
        x++;
        seek += 2;
    }
    glEnd();
    //画出放歌词的矩形
    glColor4f(1.0f,0.0f,1.0f, 0.3f);
    for (int var = 0; var < m_lrcPosItems.length(); ++var)
    {
        if(m_lrcPosItems[var].string != "")
        {
            glRectf(float(m_lrcPosItems[var].rect.x() - w / 2) / (w / 2), -1.0, float(m_lrcPosItems[var].rect.right() - w / 2) / (w / 2), 1.0);
        }
    }
    //画出歌词分界线
    glBegin(GL_LINES);
    glColor3f(1.0f,0.0f,1.0f);
    for (int var = 0; var < m_lrcPosItems.length(); ++var)
    {
        //qDebug() << m_lrcPosItems[var].string;
        if(m_lrcPosItems[var].string != "")
        {
            glVertex2f(float(m_lrcPosItems[var].rect.x() - w / 2) / (w / 2),1.0);
            glVertex2f(float(m_lrcPosItems[var].rect.x() - w / 2) / (w / 2),-1.0);
            glVertex2f(float(m_lrcPosItems[var].rect.right() - w / 2) / (w / 2),1.0);
            glVertex2f(float(m_lrcPosItems[var].rect.right() - w / 2) / (w / 2),-1.0);
        }
    }
    glEnd();
    //画出歌词
    QPainter painter(this);
    QFont font = painter.font();
    font.setPixelSize(25);
    painter.setFont(font);
    painter.setPen(QPen(QColor(255,100,255)));
    for (int var = 0; var < m_lrcPosItems.length(); ++var)
    {
        //qDebug() << m_lrcPosItems[var].string;
        if(m_lrcPosItems[var].string != "")
        {
            QRect d_rect;
            if(m_lrcPosItems[var].rect.right() - m_lrcPosItems[var].rect.left() > 10)
            {
                d_rect.setLeft(m_lrcPosItems[var].rect.left() + 5);
                d_rect.setRight(m_lrcPosItems[var].rect.right() - 5);
                d_rect.setTop(5);
                d_rect.setBottom(200-5);
                painter.drawText(d_rect, 0, m_lrcPosItems[var].string);
            }
        }
    }
    painter.end();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //未播放降低亮度
    glColor4f(0.0f,0.0f,0.0f, 0.3f);
    glRectf(0, -1.0, 1.0, 1.0);
    //画出中线
    glBegin(GL_LINES);
    glColor3f(0.0f,0.0f,1.0f);
    glVertex2f(0.0,-1.0);
    glVertex2f(0.0,1.0);
    glEnd();
}
