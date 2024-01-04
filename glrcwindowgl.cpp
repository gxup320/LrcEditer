#include "glrcwindowgl.h"
#include <QTimer>
#include <QPainter>
#include <QSurfaceFormat>
#include "glrc.h"
QT_BEGIN_NAMESPACE
extern Q_WIDGETS_EXPORT void qt_blurImage( QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0 );
QT_END_NAMESPACE

GLrcWindowGL::GLrcWindowGL(QWidget *parent)
    :QOpenGLWidget(parent)
{
    QSurfaceFormat format;
    format.setSamples(16);
    setFormat(format);
    colors = new QColor[4];
    colors[0] = QColor(0,0,0);
    colors[1] = QColor(255,0,0);
    colors[2] = QColor(0,255,0);
    colors[3] = QColor(0,0,255);
    backgroundColor = new QColor(255,255,255);
    backgroundImage = new QImage;
    backgroundPixmap = new QPixmap;
    backgroundMode = 0;
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(update()));
    m_timer->start(0);
}

GLrcWindowGL::~GLrcWindowGL()
{
    m_timer->stop();
    disconnect(m_timer, SIGNAL(timeout()), this, SLOT(update()));
    delete m_timer;
    delete[] colors;
    delete backgroundColor;
    delete backgroundImage;
    delete backgroundPixmap;
}

void GLrcWindowGL::setLrc(GLrc *lrc)
{
    m_lrc = lrc;
    m_disableMovingPicture = true;
    if(copyed != nullptr)
    {
        copyed->setLrc(lrc);
    }
}

void GLrcWindowGL::setTime(qint64 time)
{
    m_time = time;
    if(copyed != nullptr)
    {
        copyed->setTime(time);
    }
}

void GLrcWindowGL::discopy()
{
    copyed = nullptr;
}

void GLrcWindowGL::setDispaleColor(const QColor &_default, const QColor &_selectLine, const QColor &_selectLineOver, const QColor &_selectWord)
{
    colors[0] = _default;
    colors[1] = _selectLine;
    colors[2] = _selectLineOver;
    colors[3] = _selectWord;
    if(copyed != nullptr)
    {
        copyed->setDispaleColor(_default,_selectLine,_selectLineOver,_selectWord);
    }
}

void GLrcWindowGL::setBackground(QColor color)
{
    *backgroundColor = color;
    backgroundMode = 0;
    if(copyed != nullptr)
    {
        copyed->setBackground(color);
    }
}

void GLrcWindowGL::setBackground(QImage image)
{
    *backgroundImage = image;
    backgroundImageToPixmap();
    m_dark = 0;
    backgroundMode = 1;
    if(copyed != nullptr)
    {
        copyed->setBackground(image);
    }
}

void GLrcWindowGL::copyTo(GLrcWindowGL *target)
{
    //复制到目标
    copyed = target;
    if(copyed != nullptr)
    {
        //复制背景
        switch (backgroundMode) {
        case 0:
            copyed->setBackground(*backgroundColor);
            break;
        case 1:
            copyed->setBackground(QImage(*backgroundImage));
            break;
        default:
            copyed->setBackground(*backgroundColor);
            break;
        }
        target->m_dark = m_dark;
        //复制前景
        setDispaleColor(colors[0],colors[1],colors[2],colors[3]);
        //复制歌词
        copyed->setLrc(m_lrc);
        //复制时间
        copyed->setTime(m_time);
    }
    connect(target, SIGNAL(closeLrc()), this, SLOT(discopy()));
}

void GLrcWindowGL::pause(bool p)
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

int GLrcWindowGL::movSpeed(int length)
{
    if(height() / 500 < 1)
    {
        m_speed++;
    }
    else
    {
        m_speed += height() / 500;
    }
    int n_speed = 0;
    int sum = 0;
    while(sum < length)
    {
        if(height() / 500 < 1)
        {
            n_speed++;
        }
        else
        {
            n_speed += height() / 500;
        }
        sum += n_speed;
    }
    m_speed = std::min(m_speed, n_speed);
    //qDebug() << "speed:" << s_speed;
    return m_speed;
}

void GLrcWindowGL::paintEvent(QPaintEvent *e)
{    //qDebug() << time;
    //QMutexLocker locker(updateMutex);
    if(!isValid())
    {
        return;
    }
    int w = width();
    int h = height();
    QPainter painter(this);
    if(!painter.isActive())
        return;
    switch (backgroundMode) {
    case 0:
        painter.fillRect(0,0,w,h,*backgroundColor);
        break;
    case 1:
        if(!backgroundPixmap->isNull())
        {
            painter.drawPixmap(0,0,backgroundPixmap->scaled(w,h,Qt::IgnoreAspectRatio));
        }
        else
        {
            painter.fillRect(0,0,w,h,QColor(0,0,0));
        }
        painter.fillRect(QRect(0, 0, w, h),QBrush(QColor(0, 0, 0, m_dark)));
        break;
    default:
        painter.fillRect(0,0,w,h,QColor(255,255,255));
        break;
    }
    if(m_lrc != nullptr)
    {
        GLrc l_lrc = *m_lrc;
        QList<GLrc::lrcItem> l_lrcItems = l_lrc.getAllItems();
        int fountSize = l_lrc.getTextSize(w, h);
        if(fountSize <= 0)
        {
            //locker.unlock();
            return;
        }
        QList<int> selectLine, selectWord, wordLength, wordSize;
        QList<qint64> startTime, endTime;
        l_lrc.status(m_time, &selectLine, &selectWord, &wordLength, &wordSize, &startTime, &endTime);
        //painter.begin(&image);
        //painter.setBackground(QBrush(QColor(255,255,255)));
        //painter.drawRect(0,0,w,h);
        QFont font = painter.font();
        //fountSize /= 2;
        QPoint pos = {0, 0};
        bool moveed = false;
        for (int var = 0; var < l_lrcItems.length(); ++var)
        {
            //判断这一行是否被选中
            int n_selID = -1;
            for (int id = 0; id < selectLine.size(); ++id)
            {
                if(var == selectLine[id])
                {
                    n_selID = id;
                    break;
                }
            }
            if(n_selID != -1)
            {
                //当前行被选中
                if(l_lrcItems[var].fontSize < 0)
                    l_lrcItems[var].fontSize++;
                int sizeDiff = fountSize * l_lrcItems[var].fontSize / 15;
                font.setPixelSize(fountSize + sizeDiff);
                QFontMetrics fm(font);
                QStringList sl = l_lrcItems[var].line.toStringList(false);
                if(moveed == false)
                {
                    moveed = true;
                    int y2 = pos.y() + (fm.height() * (sl.size())) / 2;
                    y2 += (fm.height() - (fountSize + sizeDiff));
                    y2 = h / 2 - y2;
                    int subNum = movSpeed(abs(m_lrcY - y2));
                    if(m_lrcY > y2)
                    {
                        m_lrcY -= subNum;
                        if(m_lrcY < y2)
                        {
                            m_lrcY = y2;
                        }
                    }
                    else if(m_lrcY < y2)
                    {
                        m_lrcY += subNum;
                        if(m_lrcY > y2)
                        {
                            m_lrcY = y2;
                        }
                    }
                    if(m_disableMovingPicture || m_dark == 0)
                    {
                        m_lrcY = y2;
                        m_disableMovingPicture = false;
                    }
                }

            }
            else
            {
                //当前行未被选中，直接输出文本
                if(l_lrcItems[var].fontSize > -5)
                    l_lrcItems[var].fontSize--;
                int sizeDiff = fountSize * l_lrcItems[var].fontSize / 15;
                font.setPixelSize(fountSize + sizeDiff);
                QFontMetrics fm(font);
                QStringList sl = l_lrcItems[var].line.toStringList(false);
                pos.setY(pos.y() + fm.height() * sl.size());
            }
            pos.setY(pos.y() + fountSize / 2);
        }
        if(selectLine.size() == 0)
        {
            int y2 = h / 2 + fountSize;
            int subNum = movSpeed(abs(m_lrcY - y2));
            if(m_lrcY > y2)
            {
                m_lrcY -= subNum;
                if(m_lrcY < y2)
                {
                    m_lrcY = y2;
                }
            }
            else if(m_lrcY < y2)
            {
                m_lrcY += subNum;
                if(m_lrcY > y2)
                {
                    m_lrcY = y2;
                }
            }
            if(m_disableMovingPicture)
            {
                m_lrcY = y2;
                m_disableMovingPicture = false;
            }
        }
        pos.setY(m_lrcY);
        bool textExists = false;
        for (int var = 0; var < l_lrcItems.length(); ++var)
        {
            int sizeDiff = fountSize * l_lrcItems[var].fontSize / 15;
            //判断这一行是否被选中
            int n_selID = -1;
            for (int id = 0; id < selectLine.size(); ++id)
            {
                if(var == selectLine[id])
                {
                    n_selID = id;
                    break;
                }
            }
            if(n_selID != -1)
            {
                //当前行被选中
                font.setPixelSize(fountSize + sizeDiff);
                painter.setFont(font);
                QStringList sl = l_lrcItems[var].line.toStringList(false);
                bool fast = true;
                QFontMetrics fm(font);
                //for (auto& itm : sl)
                for(int var = 0; var < sl.length(); var++)
                {
                    auto& itm = sl[var];
                    //计算这一行宽度，计算横坐标
                    pos.setY(pos.y() + fm.height());
                    if(pos.y() > 0 && pos.y() < h + fountSize + sizeDiff && itm.length() > 0)
                    {
                        pos.setX((w - fm.horizontalAdvance(itm)) / 2);
                        QColor l_color = colors[1];
                        l_color.setAlpha(l_color.alpha() * m_dark / 100);
                        painter.setPen(l_color);
                        painter.drawText(pos, itm);
                        textExists = true;
                    }
                    if(fast && selectWord[n_selID] >= 0 && wordLength[n_selID] > 0 && startTime[n_selID] > 0 && endTime[n_selID] > 0)
                    {
                        fast = false;
                        //进度
                        int n_w = fm.horizontalAdvance(itm.mid(0, selectWord[n_selID]));
                        int n_w2 = fm.horizontalAdvance(itm.mid(selectWord[n_selID], wordLength[n_selID]));
                        qint64 t1 = m_time - startTime[n_selID];
                        qint64 t2 = endTime[n_selID] - startTime[n_selID];
                        int n_w3 = n_w + n_w2 * t1 / t2;
                        if(n_w3 > 0 && fm.height() > 0)
                        {
                            QPixmap n_image_sel(n_w2, fm.height());
                            n_image_sel.fill(*backgroundColor);
                            QPainter n_painter_sel(&n_image_sel);
                            if(backgroundMode == 1)
                            {
                                n_painter_sel.drawPixmap(QPoint(0,0),*backgroundPixmap,QRect(pos.x() + n_w, pos.y() - (fountSize + sizeDiff), n_image_sel.width(), n_image_sel.height()));
                                n_painter_sel.fillRect(QRect(0, 0, n_image_sel.width(), n_image_sel.height()),QBrush(QColor(0, 0, 0, m_dark)));
                            }
                            n_painter_sel.setFont(font);
                            QColor l_color = colors[3];
                            l_color.setAlpha(l_color.alpha() * m_dark / 100);
                            n_painter_sel.setPen(l_color);
                            QPoint n_pos_sel = {0, fountSize + sizeDiff};
                            n_painter_sel.drawText(n_pos_sel, itm.mid(selectWord[n_selID], wordLength[n_selID]));
                            painter.drawPixmap(QRect(pos.x() + n_w, pos.y() - (fountSize + sizeDiff), n_image_sel.width(), n_image_sel.height()), n_image_sel);
                            QPixmap n_image(n_w3, fm.height());
                            n_image.fill(*backgroundColor);
                            QPainter n_painter(&n_image);
                            if(backgroundMode == 1)
                            {
                                n_painter.drawPixmap(QPoint(0,0),*backgroundPixmap,QRect(pos.x(), pos.y() - (fountSize + sizeDiff), n_image.width(), n_image.height()));
                                n_painter.fillRect(QRect(0, 0, n_image.width(), n_image.height()),QBrush(QColor(0, 0, 0, m_dark)));
                            }
                            n_painter.setFont(font);
                            l_color = colors[2];
                            l_color.setAlpha(l_color.alpha() * m_dark / 100);
                            n_painter.setPen(l_color);
                            QPoint n_pos = {0, fountSize + sizeDiff};
                            n_painter.drawText(n_pos, itm);
                            textExists = true;
                            painter.drawPixmap(QRect(pos.x(), pos.y() - (fountSize + sizeDiff), n_image.width(), n_image.height()), n_image);
                        }
                    }
                }
            }
            else
            {
                //当前行未被选中，直接输出文本
                font.setPixelSize(fountSize + sizeDiff);
                painter.setFont(font);
                QFontMetrics fm(font);
                QStringList sl = l_lrcItems[var].line.toStringList(false);
                //for (auto& itm : sl)
                for(int var = 0; var < sl.length(); var++)
                {
                    auto& itm = sl[var];
                    //计算这一行宽度，计算横坐标
                    pos.setY(pos.y() + fm.height());
                    if(pos.y() > 0 && pos.y() < h + fm.height()  && itm.length() > 0)
                    {
                        QFontMetrics fm(font);
                        pos.setX((w - fm.horizontalAdvance(itm)) / 2);
                        QColor l_color = colors[0];
                        l_color.setAlpha(l_color.alpha() * m_dark / 100);
                        painter.setPen(l_color);
                        painter.drawText(pos, itm);
                        textExists = true;
                    }
                }
            }
            pos.setY(pos.y() + fountSize / 2);
        }
        if(textExists && m_dark < 100)
        {
            m_dark++;
        }
        else if(!textExists && m_dark > 0)
        {
            m_dark--;
        }
    }
}

void GLrcWindowGL::backgroundImageToPixmap()
{
    if(!isValid())
    {
        return;
    }
    QImage image = *backgroundImage;
    //labelSize是窗口尺寸
    QPixmap pxDst(size());
    QPixmap temp = QPixmap::fromImage(image).scaled(width() * 0.9,height() * 0.9, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    image = image.scaled(size(), Qt::IgnoreAspectRatio);
    pxDst.fill( Qt::transparent );
    {
        QPainter painter( &pxDst );
        qt_blurImage( &painter, image, std::max(width() / 10, height() / 10), true, false );//blur radius: 80px
    }
    //居中画出图片
    QPainter painter( &pxDst );
    painter.drawPixmap((width() - temp.width()) / 2, (height() - temp.height()) / 2,temp.width(), temp.height(), temp);
    *backgroundPixmap = pxDst;
}

void GLrcWindowGL::resizeEvent(QResizeEvent *e)
{
    QOpenGLWidget::resizeEvent(e);
    if(backgroundMode == 1)
    {
        backgroundImageToPixmap();
    }
    m_disableMovingPicture = true;
}

void GLrcWindowGL::closeEvent(QCloseEvent *event)
{
    emit closeLrc();
    QOpenGLWidget::closeEvent(event);
}
