#include "gpcmbar.h"
#include <QBuffer>
#include <QPainter>
#include <QPixmap>
#include <QMouseEvent>
#include <QThread>
#include "glrc.h"

GPcmbar::GPcmbar(QWidget *parent)
    :QLabel(parent)
{
    m_showImage = new QPixmap;
    m_showImageNext = new QPixmap;
    m_pcmImage = new QPixmap;
    m_lrcImage = new QPixmap;
    m_lrcShowImage = new QPixmap;
    setMouseTracking(true);
    connect(this, SIGNAL(drawPcmCached()), this, SLOT(displayPcm()));
    threadRunning = true;
    pcmThread = QThread::create(pcmDispaleThread, this);
    pcmThread->start();
}

GPcmbar::~GPcmbar()
{
    threadRunning = false;
    if(pcmThread != nullptr)
    {
        for(int i = 0; i < 100 && pcmThread->isRunning(); i++) thread()->msleep(100);
        if(pcmThread->isRunning())
        {
            qDebug() << "GLrc:listen stop time out,to terminate...";
            pcmThread->terminate();
        }
        while (pcmThread->isRunning()) { thread()->msleep(1);}
        delete pcmThread;
    }
}

void GPcmbar::setPcm(QByteArray pcm)
{
    m_length = pcm.length() / 2 / 441;
    QBuffer buffer;
    buffer.setBuffer(&pcm);
    buffer.open(QBuffer::ReadOnly);
    //每秒20像素
    *m_pcmImage = QPixmap(m_length, 200);
    m_pcmImage->fill(Qt::black);
    *m_lrcImage = *m_pcmImage;
    m_lrcImage->fill(QColor(0,0,0,0));
    //画出PCM
    QPainter painter(m_pcmImage);
    painter.setPen(QPen(QColor(0,255,0)));
    QLine line;
    line.setLine(0,100,0,100);
    QPoint point = line.p2();
    while(!buffer.atEnd())
    {
        QList<short> list;
        for (int var = 0; var < 441; ++var)
        {
            QByteArray num = buffer.read(2);
            if(num.size() == 2)
            {
                short y = (short(num[1]) << 8) | num[0];
                list << y;
            }
        }
        int yMax = 0;
        int yMin = 0;
        if(list.length() != 0)
        {
            yMax = *std::max_element(list.begin(), list.end());
            yMin = *std::min_element(list.begin(), list.end());
        }
        point.setY(100 + yMax * 100 / 0x7FFF);
        line.setP1(point);
        point.setY(100 + yMin * 100 / 0x7FFF);
        line.setP2(point);
        painter.drawLine(line);
        int x = line.x1();
        x++;
        line.setLine(x,100,x,100);
        point.setX(x);
    }
}

void GPcmbar::setTime(qint64 time)
{
    //将时间转换为偏移像素
    setPos(time / 5);
}

void GPcmbar::setPos(qint64 pos)
{
    if(pos != m_pos)
    {
        m_pos = pos;
        //if(m_pause == false)
            //drawPcm();
    }
}

void GPcmbar::setLrc(GLrc* lrc)
{
    m_lrc = lrc;
}

void GPcmbar::drawPcm()
{
    if(m_pause == false)
        m_displayPos = m_pos;
    if(m_showImageNext->width() == 0)
        return;
    int mid = m_showImageNext->width() / 2;
    m_showImageNext->fill(Qt::black);
    //画出底部横线
    QPainter painter(m_showImageNext);
    if(painter.isActive())
    {
        painter.setPen(QPen(QColor(0,255,0)));
        QLine line;
        line.setLine(0,100,m_showImageNext->width(),100);
        painter.drawLine(line);
        //画出PCM
        painter.drawPixmap(QPoint(mid - m_displayPos,0), *m_pcmImage);
        //画出LRC
        //painter.drawPixmap(QPoint(mid - pos,0), *m_lrcImage);
        if(m_onLrcItem == -1)
            formatLrc();
        drowLrc();
        painter.drawPixmap(QPoint(0,0), *m_lrcShowImage);
        //画出中心线
        painter.setPen(QPen(QColor(0,0,255)));
        line.setLine(mid,0,mid,200);
        painter.drawLine(line);
        painter.setPen(QPen(QColor(0,0,0, 100)));
        painter.setBrush(QBrush(QColor(0,0,0, 100)));
        painter.drawRect(QRect(mid,0,mid,200));
        if(displayed)
        {
            std::swap(m_showImage, m_showImageNext);
            emit drawPcmCached();
        }
    }
}

void GPcmbar::displayPcm()
{
    setPixmap(*m_showImage);
    displayed = true;
}

void GPcmbar::formatLrc()
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
    qint64 time = (qint64)(m_displayPos - m_lrcShowImage->width() / 2) * 5;
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
    while(l_rect.left() < m_lrcShowImage->width())
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

void GPcmbar::drowLrc()
{
    m_lrcShowImage->fill(QColor(0,0,0,0));
    QPainter painter(m_lrcShowImage);
    for (int var = 0; var < m_lrcPosItems.length(); ++var)
    {
        //qDebug() << m_lrcPosItems[var].string;
        if(m_lrcPosItems[var].string != "")
        {
            painter.setPen(QPen(QColor(255,0,0)));
            painter.setBrush(QBrush(QColor(0,0,0, 50)));
            painter.drawRect(m_lrcPosItems[var].rect);
            painter.setPen(QPen(QColor(255,100,255)));
            QRect d_rect;
            if(m_lrcPosItems[var].rect.right() - m_lrcPosItems[var].rect.left() > 10)
            {
                d_rect.setLeft(m_lrcPosItems[var].rect.left() + 5);
                d_rect.setRight(m_lrcPosItems[var].rect.right() - 5);
                d_rect.setTop(5);
                d_rect.setBottom(200-5);
                painter.drawText(d_rect,m_lrcPosItems[var].string);
            }
        }
    }
}

void GPcmbar::resizeEvent(QResizeEvent *event)
{
    *m_showImage = QPixmap(width(),200);
    *m_showImageNext = *m_showImage;
    *m_lrcShowImage = *m_showImage;
    //drawPcm();
    QLabel::resizeEvent(event);
}

void GPcmbar::enterEvent(QEnterEvent *event)
{
    m_pause = true;
    m_pausePos = m_displayPos;
}

void GPcmbar::leaveEvent(QEvent *event)
{
    m_pause = false;
    m_onLrcItem = -1;
    //drawPcm();
}

void GPcmbar::mousePressEvent(QMouseEvent *event)
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

void GPcmbar::mouseMoveEvent(QMouseEvent *event)
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
        drawPcm();
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
        drawPcm();
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

void GPcmbar::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton && m_moveing != -1 && m_displayPos != m_pausePos && m_onLrcItem == -1)
    {
        //释放鼠标，发出移动消息
        m_pos = m_displayPos;
        m_pausePos = m_displayPos;
        emit valFromMouse(m_displayPos * 5);
    }
    //qDebug() << m_onLrcItem << m_moveing << m_lrcPos << m_lrcPosItems[m_onLrcItem].rect.x();
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

void GPcmbar::pcmDispaleThread(GPcmbar *pcmbar)
{
    while(pcmbar->threadRunning)
    {
        QDateTime current_date_time = QDateTime::currentDateTime();
        if(pcmbar->m_pause == false)
        {
            pcmbar->drawPcm();
        }
        int timeout = QDateTime::currentDateTime().toMSecsSinceEpoch() - current_date_time.toMSecsSinceEpoch();
        //qDebug() << timeout;
        if(timeout < 17)
        {
            QThread::msleep(17 - timeout);
        }
    }
}
