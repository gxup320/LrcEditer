#include "glrc.h"
#include <QTime>
#include <QStringList>
#include <QPainter>
#include <QLabel>
#include <QThread>
#include <QChar>
//#include <QMutex>
//#include <QMutexLocker>

GLrc::GLrc(QObject *parent)
    : QObject{parent}
{
    colors = new QColor[4];
    colors[0] = Qt::black;
    colors[1] = Qt::red;
    colors[2] = Qt::green;
    colors[3] = Qt::blue;
    //updateMutex = new QMutex;
    threadRunning = true;
    lrcThread = QThread::create(lrcDispaleThread, this);
    lrcThread->start();
}

GLrc::GLrc(GLrc &cp)
{
    colors = new QColor[4];
    colors[0] = cp.colors[0];
    colors[1] = cp.colors[1];
    colors[2] = cp.colors[2];
    colors[3] = cp.colors[3];
    lrcItems = cp.lrcItems;
    //updateMutex = new QMutex;
    threadRunning = true;
    //lrcThread = QThread::create(lrcDispaleThread, this);
    //lrcThread->start();
}

GLrc::~GLrc()
{
    threadRunning = false;
    if(lrcThread != nullptr)
    {
        for(int i = 0; i < 100 && lrcThread->isRunning(); i++) thread()->msleep(100);
        if(lrcThread->isRunning())
        {
            qDebug() << "GLrc:listen stop time out,to terminate...";
            lrcThread->terminate();
        }
        while (lrcThread->isRunning()) { thread()->msleep(1);}
        delete lrcThread;
    }
    //delete updateMutex;
}

void GLrc::setLrc(QString lrc, int maxLine)
{
    //QMutexLocker locker(updateMutex);
    if(maxLine < 1)
        maxLine = 2147483647;
    lrcItems.resize(0);
    lrc.replace("\r\n", "\n");
    lrc.replace("\r", "\n");
    QStringList lrcLines = lrc.split("\n");
    lrcItem lrcLine;
    int lineSum = 0;
    QList<qint64> lrcTimes;
    //for(auto itm:lrcLines)
    for(int var = 0; var < lrcLines.length(); var++)
    {
        auto& itm = lrcLines[var];
        qint64 time = getTimeOfLrcLine(itm);
        QList<qint64> times;
        while(time != -1)
        {
            times << time;
            time = getTimeOfLrcLine(itm);
        }
        if((times.size() == 0 || times == lrcTimes) && lineSum < maxLine)
        {
            if(lrcLine.line.isSpace())
                lrcLine.line = itm;
            else
                lrcLine.line.append(itm);
            lineSum++;
        }
        else
        {
            lineSum = 1;
            //qDebug() << lrcTimes << lrcLine.line;
            if(lrcTimes.size() != 0 || lrcLine.line.isSpace() == false)
            {
                //qDebug() << lrcTimes << lrcLine.line;
                lrcLine.times = lrcTimes;
                lrcItems << lrcLine;
                lrcTimes.resize(0);
            }
            lrcTimes = times;
            lrcLine.line = itm;
        }
    }
    if(lrcLine.line.isSpace() == false || lrcLine.times.size() != 0)
    {
        lrcLine.times = lrcTimes;
        lrcItems << lrcLine;
    }
    selectTime = 0;
    selectLine = 0;
    //prLrcSort();
    //for(auto& itm:lrcItems)
    //{
    //    QString time;
    //    for(auto i:itm.times)
    //    {
    //        QTime t = QTime::fromMSecsSinceStartOfDay(i);
    //        QString ts = t.toString("mm:ss.") + t.toString("zzz").left(2);
    //        time += "[" + ts + "]";
    //    }
    //    //qDebug() << time << itm.line;
    //}
    //locker.unlock();
    emit lrcChanged();
}

QString GLrc::getLrc(bool moreTime /*= false*/)
{
    QString lrcFile;
    //for(auto& itm:lrcItems)
    for(int var = 0; var < lrcItems.length(); var++)
    {
        auto& itm = lrcItems[var];
        QString times = "";
        //for(auto i:itm.times)
        for (int var = 0; var < itm.times.length(); ++var)
        {
            auto i = itm.times[var];
            QTime t = QTime::fromMSecsSinceStartOfDay(i);
            QString ts = t.toString("mm:ss.") + t.toString("zzz").left(2);
            times += "[" + ts + "]";
        }
        if(moreTime)
        {
            QStringList lines = itm.line.toStringList();
            //for(auto& line:lines)
            for(int var = 0; var < lines.length(); var++)
            {
                auto& line = lines[var];
                lrcFile += times + line + "\n";
            }
        }
        else
        {
            lrcFile += times + itm.line.toString() + "\n";
        }
    }
    return lrcFile;
}

QString GLrc::getHtml(qint64 time,bool includTimes, qint64* line, int* pos, bool wordColor)
{
    qint64 localTime = getLrcTime(time);
    if(pos != nullptr)
    {
        *pos = 0;
    }
    QList<lrcItem> l_lrcItems = lrcItems;
    QString html;
    if(wordColor)
    {
        html = "<html><head><style>em{color: red;font-style:normal;}strong{color: #6A0DAD;font-style:normal;}.selete{color: green;}.w1{background-color: #008800;}.w2{background-color: #5500FF;}.w3{background-color: #55CCFF;}.wordS{color: #FFFFFF;background-color: #000000;}</style></head><body>";
    }
    else
    {
        html = "<html><head><style>em{color: red;font-style:normal;}strong{color: #6A0DAD;font-style:normal;}.selete{color: green;}.wordS{color: #FFFFFF;background-color: #000000;}</style></head><body>";
    }
    //selectTime = localTime;
    qint64 rstTime = localTime;
    bool t = true;
    for(int index = 0; index < l_lrcItems.size(); index++)
    {
        if(t && pos != nullptr)
        {
            *pos += l_lrcItems[index].line.toString().count("\n") + 1 + includTimes;
        }
        int n = -1;
        QString timesHtml = "->";
        for(int i = 0; i < l_lrcItems[index].times.size(); i++)
        {
            QTime t = QTime::fromMSecsSinceStartOfDay(l_lrcItems[index].times[i]);
            QString ts = t.toString("mm:ss.") + t.toString("zzz").left(2);
            if(l_lrcItems[index].times[i] == localTime || (time == -1 && index == selectLine && i == selectTime))
            {
                n = i;
                timesHtml+=R"(<span class="selete">[)" + ts + R"(]</span>)";
                selectTime = i;
                selectLine = index;
                if(time != -1)
                {
                    l_lrcItems[index].line.selectTime(time);
                    if(l_lrcItems[index].line.getTime() > 0)
                    {
                        rstTime = l_lrcItems[index].line.getTime();
                    }
                }
            }
            else
            {
                timesHtml+="[" + ts + "]";
            }
        }
        if(n != -1 || (time == -1 && index == selectLine))
        {
            if(includTimes)
            {
                html += "<pre><em>" + timesHtml + "\n" + l_lrcItems[index].line.toHtml(true) + "</em></pre>";
            }
            else
            {
                html += "<pre><em>" + l_lrcItems[index].line.toHtml(true) + "</em></pre>";
            }
            t = false;
        }
        else
        {
            if(includTimes)
            {
                html += "<pre>" + timesHtml + "\n" + l_lrcItems[index].line.toHtml(false) + " </pre>";
            }
            else
            {
                html += "<pre>" + l_lrcItems[index].line.toHtml(false) + "</pre>";
            }
        }
    }
    html += "</body></html>";
    if(line != nullptr)
    {
        *line = rstTime;
    }
    if(t && pos != nullptr)
    {
        *pos = 0;
    }
    //qDebug() << html;
    return html;
}

QString GLrc::getHtmlFx(qint64 time,bool includTimes, qint64* line, int* pos, bool wordColor)
{
    qint64 localTime = getLrcTime(time);
    if(pos != nullptr)
    {
        *pos = 0;
    }
    QString html;
    if(wordColor)
    {
        html = "<html><head><style>div{font-size: 20px;}em{color: red;font-style:normal;}strong{color: #6A0DAD;font-style:normal;}.selete{color: green;}.w1{background-color: #008800;}.w2{background-color: #5500FF;}.w3{background-color: #55CCFF;}.wordS{color: #FFFFFF;background-color: #000000;}</style></head><body>";
    }
    else
    {
        html = "<html><head><style>div{font-size: 20px;}em{color: red;font-style:normal;}strong{color: #6A0DAD;font-style:normal;}.selete{color: green;}.wordS{color: #FFFFFF;background-color: #000000;}</style></head><body>";
    }
    //selectTime = localTime;
    qint64 rstTime = localTime;
    bool t = true;
    QStringList lineList;
    int activation = 0;
    for(int index = 0; index < lrcItems.size(); index++)
    {
        if(t && pos != nullptr)
        {
            *pos += lrcItems[index].line.toString().count("\n") + 1 + includTimes;
        }
        int n = -1;
        QString timesHtml = "->";
        for(int i = 0; i < lrcItems[index].times.size(); i++)
        {
            QTime t = QTime::fromMSecsSinceStartOfDay(lrcItems[index].times[i]);
            QString ts = t.toString("mm:ss.") + t.toString("zzz").left(2);
            if(lrcItems[index].times[i] == localTime || (time == -1 && index == selectLine && i == selectTime))
            {
                n = i;
                timesHtml+=R"(<span class="selete">[)" + ts + R"(]</span>)";
                selectTime = i;
                selectLine = index;
                if(time != -1)
                {
                    lrcItems[index].line.selectTime(time);
                    if(lrcItems[index].line.getTime() > 0)
                    {
                        rstTime = lrcItems[index].line.getTime();
                    }
                }
            }
            else
            {
                timesHtml+="[" + ts + "]";
            }
        }
        if(n != -1 || (time == -1 && index == selectLine))
        {
            if(includTimes)
            {
                //html += "<pre><em>" + timesHtml + "\n" + lrcItems[index].line.toHtml(true) + "</em></pre>";
                lineList.append("<pre><em>" + timesHtml + "\n" + lrcItems[index].line.toHtml(true, 1) + "</em></pre>");
            }
            else
            {
                //html += "<pre><em>" + lrcItems[index].line.toHtml(true) + "</em></pre>";
                lineList.append("<pre><em>" + lrcItems[index].line.toHtml(true, 1) + "</em></pre>");
            }
            activation = lineList.size() - 1;
            t = false;
        }
        else
        {
            if(includTimes)
            {
                //html += "<pre>" + timesHtml + "\n" + lrcItems[index].line.toHtml(false) + " </pre>";
                lineList.append("<pre>" + timesHtml + "\n" + lrcItems[index].line.toHtml(false, 1) + " </pre>");
            }
            else
            {
                //html += "<pre>" + lrcItems[index].line.toHtml(false) + "</pre>";
                lineList.append("<pre>" + lrcItems[index].line.toHtml(false, 1) + "</pre>");
            }
        }
    }
    //加入html
    int cul = activation - 5;//开始行
    if(cul < 0)
    {
        cul = 0;
    }
    for(int i = cul; i < cul + 10; i++)
    {
        QString line;
        if(i < 0 || i >= lineList.size())
        {
            line = "<pre></pre>";
        }
        else
        {
            line = lineList[i];
        }
        html += "<div class=\"act" + QString::number(i - activation + 5) + "\">" + line + "</div>";
    }
    html += "</body></html>";
    if(line != nullptr)
    {
        *line = rstTime;
    }
    if(t && pos != nullptr)
    {
        *pos = 0;
    }
    //qDebug() << html;
    return html;
}

QString GLrc::getHtmlFx2(qint64 time,bool includTimes, qint64* line, int* pos, bool wordColor)
{
    includTimes = false;
    qint64 localTime = getLrcTime(time);
    if(pos != nullptr)
    {
        *pos = 0;
    }
    QString html;
    if(wordColor)
    {
        html = "<html><head><style>div{text-align: center;}.act0{font-size: 10px;}.act8{font-size: 10px;}.act1{font-size: 12px;}.act7{font-size: 12px;}.act2{font-size: 15px;}.act6{font-size: 15px;}.act3{font-size: 19px;}.act5{font-size: 19px;}.act4{font-size: 25px;}em{color: red;font-style:normal;}strong{color: #6A0DAD;font-style:normal;}.selete{color: green;}.w1{background-color: #008800;}.w2{background-color: #5500FF;}.w3{background-color: #55CCFF;}.wordS{color: #FFFFFF;background-color: #000000;}</style></head><body>";
    }
    else
    {
        html = "<html><head><style>div{text-align: center;}.act0{font-size: 10px;}.act8{font-size: 10px;}.act1{font-size: 12px;}.act7{font-size: 12px;}.act2{font-size: 15px;}.act6{font-size: 15px;}.act3{font-size: 19px;}.act5{font-size: 19px;}.act4{font-size: 25px;}em{color: red;font-style:normal;}strong{color: #6A0DAD;font-style:normal;}.selete{color: green;}.wordS{color: #FFFFFF;background-color: #000000;}</style></head><body>";
    }
    //selectTime = localTime;
    qint64 rstTime = localTime;
    bool t = true;
    QStringList lineList;
    int activation = 0;
    for(int index = 0; index < lrcItems.size(); index++)
    {
        if(t && pos != nullptr)
        {
            *pos += lrcItems[index].line.toString().count("\n") + 1 + includTimes;
        }
        int n = -1;
        QString timesHtml = "->";
        for(int i = 0; i < lrcItems[index].times.size(); i++)
        {
            QTime t = QTime::fromMSecsSinceStartOfDay(lrcItems[index].times[i]);
            QString ts = t.toString("mm:ss.") + t.toString("zzz").left(2);
            if(lrcItems[index].times[i] == localTime || (time == -1 && index == selectLine && i == selectTime))
            {
                n = i;
                timesHtml+=R"(<span class="selete">[)" + ts + R"(]</span>)";
                selectTime = i;
                selectLine = index;
                if(time != -1)
                {
                    lrcItems[index].line.selectTime(time);
                    if(lrcItems[index].line.getTime() > 0)
                    {
                        rstTime = lrcItems[index].line.getTime();
                    }
                }
            }
            else
            {
                timesHtml+="[" + ts + "]";
            }
        }
        if(n != -1 || (time == -1 && index == selectLine))
        {
            if(includTimes)
            {
                //html += "<pre><em>" + timesHtml + "\n" + lrcItems[index].line.toHtml(true) + "</em></pre>";
                lineList.append("<pre><em>" + timesHtml + "\n" + lrcItems[index].line.toHtml(true, 1) + "</em></pre>");
            }
            else
            {
                //html += "<pre><em>" + lrcItems[index].line.toHtml(true) + "</em></pre>";
                lineList.append("<pre><em>" + lrcItems[index].line.toHtml(true, 1) + "</em></pre>");
            }
            activation = lineList.size() - 1;
            t = false;
        }
        else
        {
            if(includTimes)
            {
                //html += "<pre>" + timesHtml + "\n" + lrcItems[index].line.toHtml(false) + " </pre>";
                lineList.append("<pre>" + timesHtml + "\n" + lrcItems[index].line.toHtml(false, 1) + " </pre>");
            }
            else
            {
                //html += "<pre>" + lrcItems[index].line.toHtml(false) + "</pre>";
                lineList.append("<pre>" + lrcItems[index].line.toHtml(false, 1) + "</pre>");
            }
        }
    }
    //加入html
    int cul = activation - 4;//开始行
    if(cul < 0)
    {
        cul = 0;
    }
    for(int i = cul; i < cul + 9; i++)
    {
        QString line;
        if(i < 0 || i >= lineList.size())
        {
            line = "<pre></pre>";
        }
        else
        {
            line = lineList[i];
        }
        html += "<div class=\"act" + QString::number(((i - activation + 4)>8)?8:((i - activation + 4))) + "\">" + line + "</div>";
    }
    html += "</body></html>";
    if(line != nullptr)
    {
        *line = rstTime;
    }
    if(t && pos != nullptr)
    {
        *pos = 0;
    }
    //qDebug() << html;
    return html;
}

QString GLrc::toSrt(qint64 maximumTime)
{
    //QMutexLocker locker(updateMutex);
    QList<lrcItem> temp;
    //for(auto& itm:lrcItems)
    for(int var = 0; var < lrcItems.length(); var++)
    {
        auto& itm = lrcItems[var];
        //for(auto i:itm.times)
        for(int var = 0; var < itm.times.length(); var++)
        {
            auto i = itm.times[var];
            lrcItem t;
            t.times << i;
            t.line = itm.line;
            temp << t;
        }
    }
    //locker.unlock();
    prLrcSort(temp);
    QString srt;
    qint64 cul = 1;
    for (int var = 0; var < temp.length(); ++var)
    {
        if(temp[var].line.toString(true) == temp[var].line.toString(false))
        {
            qint64 time1 = temp[var].times[0];
            qint64 time2;
            if(var == temp.length() - 1)
            {
                time2 = time1 + maximumTime;
            }
            else
            {
                time2 = temp[var + 1].times[0];
            }
            qint64 timeOut = time2 - time1;
            if(timeOut > maximumTime)
            {
                timeOut = maximumTime;
            }
            QTime t = QTime::fromMSecsSinceStartOfDay(time1);
            QString ts1 = t.toString("hh:mm:ss,zzz");
            t = QTime::fromMSecsSinceStartOfDay(time1 + timeOut);
            QString ts2 = t.toString("hh:mm:ss,zzz");
            srt += QString::number(cul) + "\n" + ts1 + " --> " + ts2 + "\n" + temp[var].line.toString(false) + "\n\n";
            cul++;
        }
        else
        {
            GLrcLine line;
            line = temp[var].line.toString();
            qint64 lastTime = temp[var].times[0];
            for (int i = 0; i < line.itmCount(); ++i)
            {
                QString subSrt = line.toSrt(i);
                qint64 time1 = line.getTime(i);
                qint64 time2;
                if(i == line.itmCount() - 1)
                {
                    if(var == temp.length() - 1)
                    {
                        time2 = time1 + maximumTime;
                    }
                    else
                    {
                        time2 = temp[var + 1].times[0];
                    }
                }
                else
                {
                    time2 = line.getTime(i + 1);
                }
                if(time1 < lastTime)
                {
                    time1 = lastTime;
                }
                lastTime = time1;
                qint64 timeOut = time2 - time1;
                if(timeOut > maximumTime)
                {
                    timeOut = maximumTime;
                }
                QTime t = QTime::fromMSecsSinceStartOfDay(time1);
                QString ts1 = t.toString("hh:mm:ss,zzz");
                t = QTime::fromMSecsSinceStartOfDay(time1 + timeOut);
                QString ts2 = t.toString("hh:mm:ss,zzz");
                srt += QString::number(cul) + "\n" + ts1 + " --> " + ts2 + "\n" + subSrt + "\n\n";
                cul++;
            }
        }
    }
    return srt;
}

qint64 GLrc::previousItem()
{
    if(lrcItems.size() == 0)
        return -1;
    selectTime--;
    if(selectTime < 0)
    {
        selectLine--;
        if(selectLine < 0)
        {
            selectLine = 0;
            selectTime = 0;
        }
        else
        {
            selectTime = lrcItems[selectLine].times.size() - 1;
        }
    }
    if(selectTime < 0)
    {
        selectTime = 0;
    }
    if(selectTime >= lrcItems[selectLine].times.size())
        return -1;
    return lrcItems[selectLine].times[selectTime];
}

qint64 GLrc::previousWord()
{
    lrcItems[selectLine].line.selectPrevious();
    qint64 time = lrcItems[selectLine].line.getTime();
    return time;
}

qint64 GLrc::nextItem()
{
    if(lrcItems.size() == 0)
        return -1;
    selectTime++;
    if(selectTime >= lrcItems[selectLine].times.size())
    {
        selectTime = 0;
        selectLine++;
    }
    if(selectLine >= lrcItems.size())
    {
        selectLine = lrcItems.size() - 1;
        selectTime = lrcItems[selectLine].times.size() - 1;
    }
    if(selectTime < 0)
    {
        selectTime = 0;
    }
    if(selectTime >= lrcItems[selectLine].times.size())
        return -1;
    return lrcItems[selectLine].times[selectTime];
}

qint64 GLrc::nextLine()
{
    if(lrcItems.size() == 0)
        return -1;
    selectLine++;
    selectTime = 0;
    if(selectLine >= lrcItems.size())
        selectLine = lrcItems.size() - 1;
    if(selectTime >= lrcItems[selectLine].times.size())
        return -1;
    return lrcItems[selectLine].times[selectTime];
}

qint64 GLrc::nextWord()
{
    //QMutexLocker locker(updateMutex);
    lrcItems[selectLine].line.selectNext();
    qint64 time = lrcItems[selectLine].line.getTime();
    //locker.unlock();
    //emit lrcChanged();
    return time;
}

int GLrc::getSelectLine()
{
    if(selectLine >= lrcItems.length())
        return -1;
    return selectLine;
}

qint64 GLrc::getLrcTime(qint64 time)
{
    qint64 localTime = -1;
    QList<lrcItem> l_lrcItems = lrcItems;
    //for(auto& itm:lrcItems)
    for(int var = 0; var < l_lrcItems.length(); var++)
    {
        auto& itm = l_lrcItems[var];
        //for(auto i:itm.times)
        for(int var = 0; var < itm.times.length(); var++)
        {
            auto i = itm.times[var];
            if(i > localTime && i <= time)
                localTime = i;
        }
    }
    return localTime;
}

qint64 GLrc::getLrcNextTime(qint64 time)
{
    QList<lrcItem> l_lrcItems = lrcItems;
    qint64 localTime = std::numeric_limits<qint64>::max();
    //for(auto& itm:l_lrcItems)
    for(int var = 0; var < l_lrcItems.length(); var++)
    {
        auto& itm = l_lrcItems[var];
        for(int var = 0; var < itm.times.length(); var++)
        {
            auto i = itm.times[var];
            if(i < localTime && i >= time)
                localTime = i;
        }
    }
    if(localTime == std::numeric_limits<qint64>::max())
        localTime = -1;
    return localTime;
}

qint64 GLrc::getSelectTime()
{
    if(selectLine >= lrcItems.size())
        return -1;
    if(selectTime >= lrcItems[selectLine].times.size())
        return -1;
    return lrcItems[selectLine].times[selectTime];
}

QString GLrc::getLineString(int line,bool incuudeTimes)
{
    if(line >= lrcItems.size())
        return "";
    return lrcItems[line].line.toString(incuudeTimes);
}

QList<qint64> GLrc::getLineTime(int line)
{
    if(line >= lrcItems.size())
        return QList<qint64>();
    return lrcItems[line].times;
}

QString GLrc::getTimes(int line)
{
    if(line >= lrcItems.size())
        return "";
    QString times;
    QList<qint64> l_times = lrcItems[line].times;
    //for(auto itm:(lrcItems[line].times))
    for(int var = 0; var < l_times.length(); var++)
    {
        auto itm = l_times[var];
        QTime t = QTime::fromMSecsSinceStartOfDay(itm);
        QString ts = t.toString("mm:ss.") + t.toString("zzz").left(2);
        times += "[" + ts + "]";
    }
    return times;
}

bool GLrc::setLine(int line, QString text)
{
    //QMutexLocker locker(updateMutex);
    if(line >= lrcItems.size())
        return false;
    lrcItems[line].line = text;
    //locker.unlock();
    emit lrcChanged();
    return true;
}

bool GLrc::setTimes(int line, QString times)
{
    //QMutexLocker locker(updateMutex);
    if(line >= lrcItems.size())
    {
        //locker.unlock();
        return false;
    }
    qint64 time = getTimeOfLrcLine(times);
    QList<qint64> arr;
    while(time != -1)
    {
        arr << time;
        time = getTimeOfLrcLine(times);
    }
    lrcItems[line].times = arr;
    //locker.unlock();
    emit lrcChanged();
    return true;
}

qint64 GLrc::timeAdd(qint64 offset)
{
    //QMutexLocker locker(updateMutex);
    if(selectLine >= lrcItems.size())
    {
        //locker.unlock();
        return 0;
    }
    qint64 localTime = lrcItems[selectLine].times[selectTime] + offset;
    if(localTime < 0)
    {
        localTime = 0;
    }
    lrcItems[selectLine].times[selectTime] = localTime;
    //locker.unlock();
    emit lrcChanged();
    return localTime;
}

void GLrc::timeAddAll(qint64 offset)
{
    //QMutexLocker locker(updateMutex);
    for (int i = 0; i < lrcItems.length(); ++i)
    {
        for (int j = 0; j < lrcItems[i].times.length(); ++j)
        {
            lrcItems[i].times[j] += offset;
            if(lrcItems[i].times[j] < 0)
            {
                lrcItems[i].times[j] = 0;
            }
        }
        for (int j = 0; j < lrcItems[i].line.itmCount(); ++j)
        {
            qint64 tempTime = lrcItems[i].line.getTime(j);
            if(tempTime != -1)
            {
                tempTime += offset;
                if(tempTime < 0)
                {
                    tempTime = 0;
                }
                lrcItems[i].line.setTime(tempTime, j);
            }
        }
    }
    //locker.unlock();
    emit lrcChanged();
}

qint64 GLrc::wordTimeAdd(qint64 offset)
{
    //QMutexLocker locker(updateMutex);
    if(selectLine >= lrcItems.size())
    {
        //locker.unlock();
        return 0;
    }
    qint64 localTime = lrcItems[selectLine].line.getTime() + offset;
    if(localTime < lrcItems[selectLine].times[selectTime])
    {
        localTime = lrcItems[selectLine].times[selectTime];
    }
    lrcItems[selectLine].line.setTime(localTime);
    //locker.unlock();
    emit lrcChanged();
    return localTime;
}

bool GLrc::instTime(qint64 time, int line)
{
    //QMutexLocker locker(updateMutex);
    if(lrcItems.size() == 0)
    {
        //locker.unlock();
        return false;
    }
    if(line == -1)
        line = selectLine;
    if(line >= lrcItems.size())
        return false;
    lrcItems[line].times << time;
    //locker.unlock();
    emit lrcChanged();
    return true;
}

qint64 GLrc::removeTime(int line, int itm)
{
    //QMutexLocker locker(updateMutex);
    if(line == -1)
        line = selectLine;
    if(itm == -1)
    {
        itm = selectTime;
        if(selectTime > 0)
            selectTime--;
    }
    if(line >= lrcItems.size())
    {
        //locker.unlock();
        return -1;
    }
    if(itm >= lrcItems[line].times.size())
        itm = lrcItems[line].times.size() - 1;
    if(itm < 0)
    {
        //locker.unlock();
        return -1;
    }
    qint64 time = lrcItems[line].times[itm];
    lrcItems[line].times.remove(itm);
    //locker.unlock();
    emit lrcChanged();
    return time;
}

void GLrc::mergeDuplicates(mode m)
{
    //QMutexLocker locker(updateMutex);
    int len = lrcItems.size();
    if(m == AUTO || m == MERGE)
    {
        for(int index = 0; index < lrcItems.size(); index++)
        {
            for(int i = index + 1; i < lrcItems.size(); i++)
            {
                if(lrcItems[index].line == lrcItems[i].line)
                {
                    lrcItems[index].times << lrcItems[i].times;
                    lrcItems.remove(i);
                    i--;
                }
            }
        }
    }
    if((len == lrcItems.size() && m == AUTO) || m == SPLIT)
    {
        QList<lrcItem> temp;
        //for(auto& itm:lrcItems)
        for(int var = 0; var < lrcItems.length(); var++)
        {
            auto& itm = lrcItems[var];
            //for(auto i:itm.times)
            for(int var = 0; var < itm.times.length(); var++)
            {
                auto i = itm.times[var];
                lrcItem t;
                t.times << i;
                t.line = itm.line;
                temp << t;
            }
        }
        lrcItems = temp;
        prLrcSort(lrcItems);
    }
    //locker.unlock();
    emit lrcChanged();
}

int GLrc::instLine(int line)
{
    //QMutexLocker locker(updateMutex);
    lrcItem t;
    if(line == -1)
    {
        line = selectLine;
    }
    if(line > lrcItems.size())
        line = lrcItems.size();
    lrcItems.insert(line,t);
    //locker.unlock();
    emit lrcChanged();
    return line;
}

qint64 GLrc::getTimeOfLrcLine(QString &lrcLine)
{
    if(lrcLine.left(1) != "[")
    {
        return -1;
    }
    if(lrcLine.indexOf("]") == -1)
    {
        return -1;
    }
    QString timeStr = lrcLine.mid(1,lrcLine.indexOf("]") - 1);
    //qDebug() << timeStr;
    QTime time = QTime::fromString(timeStr + "0","mm:ss.zzz");
    if(time.isNull())
    {
        time = QTime::fromString(timeStr,"mm:ss.zzz");
    }
    if(time.isNull())
    {
        return -1;
    }
    lrcLine = lrcLine.right(lrcLine.length() - lrcLine.indexOf("]") - 1);
    return time.msecsSinceStartOfDay();
}

qint64 GLrc::getSmallTime(lrcItem line)
{
    qint64 time = INT64_MAX;
    //for(auto i:line.times)
    for(int var = 0; var < line.times.length(); var++)
    {
        auto i = line.times[var];
        if(i < time)
        {
            time = i;
        }
    }
    if(time == INT64_MAX)
        time = -1;
    return time;
}

void GLrc::prLrcSort(QList<lrcItem>& items)
{
    int i, j;
    for (i = 0; i < items.length() - 1; i++)
        for (j = 0; j < items.length() - 1 - i; j++)
            if (getSmallTime(items[j]) > getSmallTime(items[j + 1]))
                items.swapItemsAt(j, j + 1);
}

void GLrc::lrcDispaleThread(GLrc *lrc)
{
    while(lrc->threadRunning)
    {
        QDateTime current_date_time = QDateTime::currentDateTime();
        lrc->updateLrcwindow(lrc->lrcDispaleTime);
        int timeout = QDateTime::currentDateTime().toMSecsSinceEpoch() - current_date_time.toMSecsSinceEpoch();
        //qDebug() << timeout;
        if(timeout < 17)
        {
            QThread::msleep(17 - timeout);
        }
    }

}

void GLrc::status(QList<lrcItem>& c_lrcItems, qint64 time,QList<int> *line, QList<int> *word,QList<int>* wordSeleteLength, QList<int> *wordSize, QList<qint64> *startTime, QList<qint64> *endTime)
{
    qint64 localTime = getLrcTime(time);
    qint64 nextTime = getLrcNextTime(localTime + 1);
    for (int var = 0; var < c_lrcItems.length(); ++var)
    {
        //for(auto& itm : c_lrcItems[var].times)
        for(int i = 0; i < c_lrcItems[var].times.length(); i++)
        {
            auto& itm = c_lrcItems[var].times[i];
            if(itm == localTime)
            {
                line->push_back(var);
                break;
            }
        }
    }
    for (int var = 0; var < line->size(); ++var)
    {
        int n_wordSize = 0,n_word = 0,n_wordSeleteLength = 0;
        qint64 n_startTime = -1, n_endTime = -1;
        n_wordSize = c_lrcItems[line->at(var)].line.status(time, &n_startTime, &n_endTime, &n_word, &n_wordSeleteLength);
        if(n_startTime == -1)
        {
            n_startTime = localTime;
        }
        if(n_endTime == -1)
        {
            n_endTime = nextTime;
        }
        word->push_back(n_word);
        wordSeleteLength->push_back(n_wordSeleteLength);
        wordSize->push_back(n_wordSize);
        startTime->push_back(n_startTime);
        endTime->push_back(n_endTime);
    }
}

int GLrc::getTextSize(QList<lrcItem>& c_lrcItems, int w, int h)
{
    if(w <= 0 || h <= 0)
    {
        return 0;
    }
    //寻找最长字符串
    QFont myFont;
    myFont.setPixelSize(10);
    QFontMetrics fm(myFont);
    int strLength = 0;
    //for(auto& itm : c_lrcItems)
    for(int var = 0; var < c_lrcItems.length(); var++)
    {
        auto& itm = c_lrcItems[var];
        QStringList sl = itm.line.toStringList(false);
        //for (auto& itm : sl)
        for(int var = 0; var < sl.length(); var++)
        {
            auto& itm = sl[var];
            if(fm.horizontalAdvance(itm) > strLength)
            {
                strLength = fm.horizontalAdvance(itm);
            }
        }
    }
    if(strLength <= 0)
        return 6;
    int size1 = w / ((strLength / 10.0) + 1);
    int size2 = h / 15;
    if(std::min(size1, size2) < 6)
        return 6;
    return std::min(size1, size2);
}

int GLrc::movSpeed(int length)
{
    static int s_speed = 0;
    s_speed++;
    int n_speed = 0;
    int sum = 0;
    while(sum < length)
    {
        n_speed++;
        sum += n_speed;
    }
    s_speed = std::min(s_speed, n_speed);
    //qDebug() << "speed:" << s_speed;
    return s_speed;
}

void GLrc::lrcTimesSort()
{
    //QMutexLocker locker(updateMutex);
    prLrcSort(lrcItems);
    //locker.unlock();
    emit lrcChanged();
}

int GLrc::size()
{
    return lrcItems.size();
}

void GLrc::setSelectLine(int line)
{
    selectLine = line;
}

void GLrc::setSeleteTime(qint64 time)
{
    selectLine = -1;
    qint64 lrcTime = getLrcTime(time);
    for (int var = 0; var < lrcItems.length(); ++var)
    {
        for (int var2 = 0; var2 < lrcItems[var].times.length(); ++var2)
        {
            if(lrcItems[var].times[var2] == lrcTime)
            {
                selectLine = var;
                break;
            }
        }
        if(selectLine != -1)
        {
            break;
        }
    }
}

bool GLrc::removeLine(int line)
{
    //QMutexLocker locker(updateMutex);
    if(lrcItems.size() == 0)
        return false;
    if(line == -1)
        line = selectLine;
    if(line >= lrcItems.size())
        return false;
    lrcItems.remove(line);
    if(line == selectLine)
    {
        selectTime = 0;
        if(line > 0)
            selectLine--;
    }
    //locker.unlock();
    emit lrcChanged();
    return true;
}

qint64 GLrc::setWordTime(qint64 time)
{
    if(selectLine >= 0 && selectLine < lrcItems.length())
    {
        //QMutexLocker locker(updateMutex);
        qint64 t = lrcItems[selectLine].line.setTime(time);
        //locker.unlock();
        emit lrcChanged();
        return t;
    }

    return -1;
}

qint64 GLrc::getWordTime()
{
    if(selectLine >= 0 && selectLine < lrcItems.length())
        return lrcItems[selectLine].line.getTime();
    return -1;
}

int GLrc::getSelectWord()
{
    return lrcItems[selectLine].line.getSelect();
}

qint64 GLrc::selectWordId(int id)
{
    return lrcItems[selectLine].line.selectId(id);
}

qint64 GLrc::deleteWordTime()
{
    //QMutexLocker locker(updateMutex);
    qint64 t = lrcItems[selectLine].line.deleteTime();
    //locker.unlock();
    if(t != -1)
    {
        emit lrcChanged();
    }
    return t;
}

int GLrc::deleteLineWordTime()
{
    //QMutexLocker locker(updateMutex);
    int rev = lrcItems[selectLine].line.deleteAllTime();
    //locker.unlock();
    if(rev != 0)
    {
        emit lrcChanged();
    }
    return rev;
}

int GLrc::deleteAllWordTime()
{
    //QMutexLocker locker(updateMutex);
    int rev = 0;
    for (int var = 0; var < lrcItems.length(); ++var)
    {
        rev += lrcItems[var].line.deleteAllTime();
    }
    //locker.unlock();
    if(rev != 0)
    {
        emit lrcChanged();
    }
    return rev;
}

int GLrc::replaceTime(qint64 sorce, qint64 target)
{
    int retv = 0;
    for (int var = 0; var < lrcItems.length(); ++var)
    {
        for (int i = 0; i < lrcItems[var].times.length(); ++i)
        {
            if(lrcItems[var].times[i] == sorce)
            {
                lrcItems[var].times[i] = target;
                retv++;
            }
        }
    }
    if(retv > 0)
    {
        emit lrcChanged(1);
    }
    return retv;
}

void GLrc::setDispaleColor(const QColor &_default, const QColor &_selectLine, const QColor &_selectLineOver, const QColor &_selectWord)
{
    colors[0] = _default;
    colors[1] = _selectLine;
    colors[2] = _selectLineOver;
    colors[3] = _selectWord;
}

QSize GLrc::setLabelSize(QSize _labelSize)
{
    //QMutexLocker locker(updateMutex);
    QSize t = labelSize;
    labelSize = _labelSize;
    m_disableMovingPicture = true;
    //locker.unlock();
    return t;
}

void GLrc::updateLrcwindow(qint64 time)
{
    //qDebug() << time;
    //QMutexLocker locker(updateMutex);
    if(labelSize.isNull() || labelSize.height() == 0 || labelSize.width() == 0)
    {
        //locker.unlock();
        return;
    }
    QList<lrcItem> l_lrcItems = lrcItems;
    int w = labelSize.width();
    int h = labelSize.height();
    int fountSize = getTextSize(l_lrcItems, w, h);
    if(fountSize <= 0)
    {
        //locker.unlock();
        return;
    }
    QList<int> selectLine, selectWord, wordLength, wordSize;
    QList<qint64> startTime, endTime;
    status(l_lrcItems,time, &selectLine, &selectWord, &wordLength, &wordSize, &startTime, &endTime);
    //qDebug() << selectLine;
    //计算画板高度
    //int maph = 0;
    //for (auto& itm : lrcItems) {
    //    maph += fountSize * 0.5;//两行间距
    //    //加上每一行文字的长度
    //    maph += itm.line.getLineSum();
    //}
    if(imageNext != nullptr)
    {
        delete imageNext;
    }
    imageNext = new QPixmap(w, h);
    imageNext->fill();
    QPainter painter(imageNext);
    //painter.begin(&image);
    //painter.setBackground(QBrush(QColor(255,255,255)));
    //painter.drawRect(0,0,w,h);
    QFont font = painter.font();
    //fountSize /= 2;
    QPoint pos = {0, 0};
    static int s_y = 0;
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
                int subNum = movSpeed(abs(s_y - y2));
                if(s_y > y2)
                {
                    s_y -= subNum;
                }
                else if(s_y < y2)
                {
                    s_y += subNum;
                }
                if(m_disableMovingPicture)
                {
                    s_y = y2;
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
        int subNum = movSpeed(abs(s_y - y2));
        if(s_y > y2)
        {
            s_y -= subNum;
        }
        else if(s_y < y2)
        {
            s_y += subNum;
        }
        if(m_disableMovingPicture)
        {
            s_y = y2;
            m_disableMovingPicture = false;
        }
    }
    pos.setY(s_y);
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
                    painter.setPen(colors[1]);
                    painter.drawText(pos, itm);
                }
                if(fast && selectWord[n_selID] >= 0 && wordLength[n_selID] > 0 && startTime[n_selID] > 0 && endTime[n_selID] > 0)
                {
                    fast = false;
                    //进度
                    int n_w = fm.horizontalAdvance(itm.mid(0, selectWord[n_selID]));
                    int n_w2 = fm.horizontalAdvance(itm.mid(selectWord[n_selID], wordLength[n_selID]));
                    qint64 t1 = time - startTime[n_selID];
                    qint64 t2 = endTime[n_selID] - startTime[n_selID];
                    int n_w3 = n_w + n_w2 * t1 / t2;
                    if(n_w3 > 0 && fm.height() > 0)
                    {
                        QPixmap n_image_sel(n_w2, fm.height());
                        n_image_sel.fill();
                        QPainter n_painter_sel(&n_image_sel);
                        n_painter_sel.setFont(font);
                        n_painter_sel.setPen(colors[3]);
                        QPoint n_pos_sel = {0, fountSize + sizeDiff};
                        n_painter_sel.drawText(n_pos_sel, itm.mid(selectWord[n_selID], wordLength[n_selID]));
                        painter.drawPixmap(QRect(pos.x() + n_w, pos.y() - (fountSize + sizeDiff), n_image_sel.width(), n_image_sel.height()), n_image_sel);
                        QPixmap n_image(n_w3, fm.height());
                        n_image.fill();
                        QPainter n_painter(&n_image);
                        n_painter.setFont(font);
                        n_painter.setPen(colors[2]);
                        QPoint n_pos = {0, fountSize + sizeDiff};
                        n_painter.drawText(n_pos, itm);
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
                    painter.setPen(colors[0]);
                    painter.drawText(pos, itm);
                }
            }
        }
        pos.setY(pos.y() + fountSize / 2);
    }
    //locker.unlock();
    //while(!imgReadEd)thread()->msleep(1);
    if(imgReadEd)
    {
        std::swap(image,imageNext);
        imgReadEd = false;
        emit lrcImgChanged();
    }
}

qint64 GLrc::setDispaleTime(qint64 time)
{
    qint64 t = lrcDispaleTime;
    lrcDispaleTime = time;
    return t;
}

void GLrc::disableMovingPicture()
{
    //QMutexLocker locker(updateMutex);
    m_disableMovingPicture = true;
    //locker.unlock();
}

const QPixmap *GLrc::getPixmap()
{
    std::swap(image,imageGet);
    imgReadEd = true;
    return imageGet;
}

