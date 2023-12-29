#include "processlabel.h"
#include <QBuffer>
#include <QPainter>
#include <QPixmap>
#include <QMouseEvent>
#include "glrc.h"

ProcessLabel::ProcessLabel(QWidget *parent)
    :QLabel(parent)
{
    m_showImage = new QPixmap(width(),200);
    m_pcmImage = new QPixmap;
    m_lrcImage = new QPixmap;
    m_lrcShowImage = new QPixmap(width(),200);
    setMouseTracking(true);
}

void ProcessLabel::setPcm(QByteArray pcm)
{
    m_length = pcm.length() / 4;
    QBuffer buffer;
    buffer.setBuffer(&pcm);
    buffer.open(QBuffer::ReadOnly);
    //每秒20像素
    *m_pcmImage = QPixmap(pcm.length() / 2 / 441, 200);
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

void ProcessLabel::setTime(qint64 time)
{
    //将时间转换为偏移像素
    setPos(time / 5);
}

void ProcessLabel::setPos(qint64 pos)
{
    if(pos != m_pos)
    {
        m_pos = pos;
        if(m_pause == false)
            drawPcm();
    }
}

void ProcessLabel::setLrc(GLrc* lrc)
{
    m_lrc = lrc;
    return;
    GLrc l_lrc = *m_lrc;
    l_lrc.mergeDuplicates(GLrc::SPLIT);
    QPainter painter(m_lrcImage);
    m_lrcImage->fill(QColor(0,0,0,0));
    for (int var = 0; var < l_lrc.size(); ++var)
    {
        QString lrcLine = l_lrc.getLineString(var, false);
        if(lrcLine == "")
        {
            continue;
        }
        QList<qint64> time = l_lrc.getLineTime(var);
        if(time.length() < 1)
        {
            continue;
        }
        qint64 nextTime = l_lrc.getLrcNextTime(time[0] + 1);
        if(nextTime == -1)
        {
            nextTime = m_length;
        }
        //画出lrc
        QPixmap lrcPixmap((nextTime - time[0]) / 5,200);
        lrcPixmap.fill(QColor(255,0,0,50));
        {
            QPainter painter(&lrcPixmap);
            painter.setPen(QPen(QColor(255,0,0)));
            painter.setBrush(QBrush(QColor(0,0,0, 0)));
            painter.drawRect(QRect(0,0,lrcPixmap.width() - 1,200));
            painter.setPen(QPen(QColor(255,100,255)));
            painter.drawText(lrcPixmap.rect(),lrcLine);
        }
        //qDebug() << lrcLine;
        painter.drawPixmap(QPoint(time[0] / 5,0), lrcPixmap);
    }
}

void ProcessLabel::drawPcm()
{
    if(m_pause == false)
        m_displayPos = m_pos;
    int mid = m_showImage->width() / 2;
    m_showImage->fill(Qt::black);
    //画出底部横线
    QPainter painter(m_showImage);
    painter.setPen(QPen(QColor(0,255,0)));
    QLine line;
    line.setLine(0,100,m_showImage->width(),100);
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
    setPixmap(*m_showImage);
}

void ProcessLabel::formatLrc()
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

void ProcessLabel::drowLrc()
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

void ProcessLabel::resizeEvent(QResizeEvent *event)
{
    *m_showImage = QPixmap(width(),200);
    *m_lrcShowImage = *m_showImage;
    drawPcm();
    QLabel::resizeEvent(event);
}

void ProcessLabel::enterEvent(QEnterEvent *event)
{
    m_pause = true;
    m_pausePos = m_displayPos;
}

void ProcessLabel::leaveEvent(QEvent *event)
{
    m_pause = false;
    m_onLrcItem = -1;
    drawPcm();
}

void ProcessLabel::mousePressEvent(QMouseEvent *event)
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

void ProcessLabel::mouseMoveEvent(QMouseEvent *event)
{
    if(m_moveing != -1 && m_onLrcItem == -1)
    {
        //移动进度条
        m_displayPos = m_pausePos + m_moveing - event->pos().x();
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

void ProcessLabel::mouseReleaseEvent(QMouseEvent *event)
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
