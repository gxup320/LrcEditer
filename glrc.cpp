#include "glrc.h"
#include <QTime>
#include <QStringList>
#include <QPainter>
#include <QLabel>
#include <QThread>
#include <QChar>
#include <QMutex>
#include <QMutexLocker>

GLrc::GLrc(QObject *parent)
    : QObject{parent}
{
    updateMutex = new QMutex;
    threadRunning = true;
    lrcThread = QThread::create(lrcDispaleThread, this);
    lrcThread->start();
}

GLrc::~GLrc()
{
    threadRunning = false;
    for(int i = 0; i < 100 && lrcThread->isRunning(); i++) thread()->msleep(100);
    if(lrcThread->isRunning())
    {
        qDebug() << "listen stop time out,to terminate...";
        lrcThread->terminate();
    }
    while (lrcThread->isRunning()) { thread()->msleep(1);}
    delete lrcThread;
    delete updateMutex;
}

void GLrc::setLrc(QString lrc, int maxLine)
{
    QMutexLocker locker(updateMutex);
    if(maxLine < 1)
        maxLine = 2147483647;
    lrcItems.resize(0);
    lrc.replace("\r\n", "\n");
    lrc.replace("\r", "\n");
    static QRegularExpression reg("\\s*+$");
    lrc.remove(reg);
    QStringList lrcLines = lrc.split("\n");
    lrcItem lrcLine;
    int lineSum = 0;
    QList<qint64> lrcTimes;
    for(auto itm:lrcLines)
    {
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
    prLrcSort();
    for(auto& itm:lrcItems)
    {
        QString time;
        for(auto i:itm.times)
        {
            QTime t = QTime::fromMSecsSinceStartOfDay(i);
            QString ts = t.toString("mm:ss.") + t.toString("zzz").left(2);
            time += "[" + ts + "]";
        }
        //qDebug() << time << itm.line;
    }
    locker.unlock();
    emit lrcChanged();
}

QString GLrc::getLrc(bool moreTime /*= false*/)
{
    QString lrcFile;
    for(auto& itm:lrcItems)
    {
        QString times = "";
        for(auto i:itm.times)
        {
            QTime t = QTime::fromMSecsSinceStartOfDay(i);
            QString ts = t.toString("mm:ss.") + t.toString("zzz").left(2);
            times += "[" + ts + "]";
        }
        if(moreTime)
        {
            QStringList lines = itm.line.toStringList();
            for(auto& line:lines)
            {
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
                html += "<pre><em>" + timesHtml + "\n" + lrcItems[index].line.toHtml(true) + "</em></pre>";
            }
            else
            {
                html += "<pre><em>" + lrcItems[index].line.toHtml(true) + "</em></pre>";
            }
            t = false;
        }
        else
        {
            if(includTimes)
            {
                html += "<pre>" + timesHtml + "\n" + lrcItems[index].line.toHtml(false) + " </pre>";
            }
            else
            {
                html += "<pre>" + lrcItems[index].line.toHtml(false) + "</pre>";
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
    QMutexLocker locker(updateMutex);
    lrcItems[selectLine].line.selectNext();
    qint64 time = lrcItems[selectLine].line.getTime();
    locker.unlock();
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
    for(auto& itm:lrcItems)
    {
        for(auto i:itm.times)
        {
            if(i > localTime && i <= time)
                localTime = i;
        }
    }
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

QString GLrc::getLine(int line)
{
    if(line >= lrcItems.size())
        return "";
    return lrcItems[line].line.toString();
}

QString GLrc::getTimes(int line)
{
    if(line >= lrcItems.size())
        return "";
    QString times;
    for(auto itm:(lrcItems[line].times))
    {
        QTime t = QTime::fromMSecsSinceStartOfDay(itm);
        QString ts = t.toString("mm:ss.") + t.toString("zzz").left(2);
        times += "[" + ts + "]";
    }
    return times;
}

bool GLrc::setLine(int line, QString text)
{
    QMutexLocker locker(updateMutex);
    if(line >= lrcItems.size())
        return false;
    lrcItems[line].line = text;
    locker.unlock();
    emit lrcChanged();
    return true;
}

bool GLrc::setTimes(int line, QString times)
{
    QMutexLocker locker(updateMutex);
    if(line >= lrcItems.size())
    {
        locker.unlock();
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
    locker.unlock();
    emit lrcChanged();
    return true;
}

qint64 GLrc::timeAdd(qint64 offset)
{
    QMutexLocker locker(updateMutex);
    if(selectLine >= lrcItems.size())
    {
        locker.unlock();
        return 0;
    }
    qint64 localTime = lrcItems[selectLine].times[selectTime] + offset;
    if(localTime < 0)
    {
        localTime = 0;
    }
    lrcItems[selectLine].times[selectTime] = localTime;
    locker.unlock();
    emit lrcChanged();
    return localTime;
}

qint64 GLrc::wordTimeAdd(qint64 offset)
{
    QMutexLocker locker(updateMutex);
    if(selectLine >= lrcItems.size())
    {
        locker.unlock();
        return 0;
    }
    qint64 localTime = lrcItems[selectLine].line.getTime() + offset;
    if(localTime < lrcItems[selectLine].times[selectTime])
    {
        localTime = lrcItems[selectLine].times[selectTime];
    }
    lrcItems[selectLine].line.setTime(localTime);
    locker.unlock();
    emit lrcChanged();
    return localTime;
}

bool GLrc::instTime(qint64 time, int line)
{
    QMutexLocker locker(updateMutex);
    if(lrcItems.size() == 0)
    {
        locker.unlock();
        return false;
    }
    if(line == -1)
        line = selectLine;
    if(line >= lrcItems.size())
        return false;
    lrcItems[line].times << time;
    locker.unlock();
    emit lrcChanged();
    return true;
}

qint64 GLrc::removeTime(int line, int itm)
{
    QMutexLocker locker(updateMutex);
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
        locker.unlock();
        return -1;
    }
    if(itm >= lrcItems[line].times.size())
        itm = lrcItems[line].times.size() - 1;
    if(itm < 0)
    {
        locker.unlock();
        return -1;
    }
    qint64 time = lrcItems[line].times[itm];
    lrcItems[line].times.remove(itm);
    locker.unlock();
    emit lrcChanged();
    return time;
}

void GLrc::mergeDuplicates()
{
    QMutexLocker locker(updateMutex);
    int len = lrcItems.size();
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
    if(len == lrcItems.size())
    {
        QList<lrcItem> temp;
        for(auto& itm:lrcItems)
        {
            for(auto i:itm.times)
            {
                lrcItem t;
                t.times << i;
                t.line = itm.line;
                temp << t;
            }
        }
        lrcItems = temp;
        prLrcSort();
    }
    locker.unlock();
    emit lrcChanged();
}

int GLrc::instLine(int line)
{
    QMutexLocker locker(updateMutex);
    lrcItem t;
    if(line == -1)
    {
        line = selectLine;
    }
    if(line > lrcItems.size())
        line = lrcItems.size();
    lrcItems.insert(line,t);
    locker.unlock();
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
    for(auto i:line.times)
    {
        if(i < time)
        {
            time = i;
        }
    }
    if(time == INT64_MAX)
        time = -1;
    return time;
}

void GLrc::prLrcSort()
{
    int i, j;
    for (i = 0; i < lrcItems.length() - 1; i++)
        for (j = 0; j < lrcItems.length() - 1 - i; j++)
            if (getSmallTime(lrcItems[j]) > getSmallTime(lrcItems[j + 1]))
                lrcItems.swapItemsAt(j, j + 1);
}

void GLrc::lrcDispaleThread(GLrc *lrc)
{
    while(lrc->threadRunning)
    {
        lrc->updateLrcwindow(lrc->lrcDispaleTime);
        QThread::msleep(10);
    }

}

void GLrc::status(qint64 time,int *line, int *word,int* wordSeleteLength, int *wordSize, qint64 *startTime, qint64 *endTime)
{
    qint64 localTime = getLrcTime(time);
    *line = -1;
    for (int var = 0; var < lrcItems.length(); ++var)
    {
        for(auto& itm : lrcItems[var].times)
        {
            if(itm == localTime)
            {
                *line = var;
                break;
            }
        }
        if(*line != -1)
        {
            *wordSize = lrcItems[*line].line.status(time, startTime, endTime, word, wordSeleteLength);
            if(*startTime == -1 && lrcItems[*line].times.size() > 0)
            {
                *startTime = lrcItems[*line].times[0];
            }
            if(*endTime == -1)
            {
                if(*line + 1 < lrcItems.size())
                {
                    if(lrcItems[*line + 1].times.size() > 0)
                    {
                        *endTime = lrcItems[*line + 1].times[0];
                    }
                }
            }
            //qDebug() << *startTime << "/" << *endTime;
            break;
        }
    }
}

int GLrc::getTextSize(int w, int h)
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
    for(auto& itm : lrcItems)
    {
        QStringList sl = itm.line.toStringList(false);
        for (auto& itm : sl)
        {
            if(fm.horizontalAdvance(itm) > strLength)
            {
                strLength = fm.horizontalAdvance(itm);
            }
        }
    }
    if(strLength <= 0)
        return 6;
    int size1 = w / ((strLength / 10.0) + 1);
    int size2 = h / 10;
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
    QMutexLocker locker(updateMutex);
    prLrcSort();
    locker.unlock();
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

bool GLrc::removeLine(int line)
{
    QMutexLocker locker(updateMutex);
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
    locker.unlock();
    emit lrcChanged();
    return true;
}

qint64 GLrc::setWordTime(qint64 time)
{
    if(selectLine >= 0 && selectLine < lrcItems.length())
    {
        QMutexLocker locker(updateMutex);
        qint64 t = lrcItems[selectLine].line.setTime(time);
        locker.unlock();
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
    QMutexLocker locker(updateMutex);
    qint64 t = lrcItems[selectLine].line.deleteTime();
    locker.unlock();
    if(t != -1)
    {
        emit lrcChanged();
    }
    return t;
}

int GLrc::deleteLineWordTime()
{
    QMutexLocker locker(updateMutex);
    int rev = lrcItems[selectLine].line.deleteAllTime();
    locker.unlock();
    if(rev != 0)
    {
        emit lrcChanged();
    }
    return rev;
}

int GLrc::deleteAllWordTime()
{
    QMutexLocker locker(updateMutex);
    int rev = 0;
    for (int var = 0; var < lrcItems.length(); ++var)
    {
        rev += lrcItems[var].line.deleteAllTime();
    }
    locker.unlock();
    if(rev != 0)
    {
        emit lrcChanged();
    }
    return rev;
}

QLabel * GLrc::setLabel(QLabel *_label)
{
    QMutexLocker locker(updateMutex);
    QLabel * t = label;
    label = _label;
    locker.unlock();
    return t;
}

void GLrc::updateLrcwindow(qint64 time)
{
    QMutexLocker locker(updateMutex);
    if(label == nullptr)
    {
        locker.unlock();
        return;
    }
    int w = label->width();
    int h = label->height();
    int fountSize = getTextSize(w, h);
    if(fountSize <= 0)
    {
        locker.unlock();
        return;
    }
    int selectLine = -1, selectWord = -1, wordLength = 0, wordSize = 0;
    qint64 startTime = -1, endTime = -1;
    status(time, &selectLine, &selectWord, &wordLength, &wordSize, &startTime, &endTime);
    //计算画板高度
    //int maph = 0;
    //for (auto& itm : lrcItems) {
    //    maph += fountSize * 0.5;//两行间距
    //    //加上每一行文字的长度
    //    maph += itm.line.getLineSum();
    //}
    QPixmap image(w, h);
    image.fill();
    QPainter painter(&image);
    //painter.begin(&image);
    //painter.setBackground(QBrush(QColor(255,255,255)));
    //painter.drawRect(0,0,w,h);
    QFont font = painter.font();
    //fountSize /= 2;
    QPoint pos = {0, 0};
    static int s_y = 0;
    for (int var = 0; var < lrcItems.length(); ++var)
    {
        //判断这一行是否被选中
        if(var == selectLine)
        {
            //当前行被选中
            if(lrcItems[var].fontSize < 0)
                lrcItems[var].fontSize++;
            int sizeDiff = fountSize * lrcItems[var].fontSize / 15;
            font.setPixelSize(fountSize + sizeDiff);
            QFontMetrics fm(font);
            QStringList sl = lrcItems[var].line.toStringList(false);
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
        else
        {
            //当前行未被选中，直接输出文本
            if(lrcItems[var].fontSize > -5)
                lrcItems[var].fontSize--;
            int sizeDiff = fountSize * lrcItems[var].fontSize / 15;
            font.setPixelSize(fountSize + sizeDiff);
            QFontMetrics fm(font);
            QStringList sl = lrcItems[var].line.toStringList(false);
            pos.setY(pos.y() + fm.height() * sl.size());
        }
        pos.setY(pos.y() + fountSize / 2);
    }
    pos.setY(s_y);
    for (int var = 0; var < lrcItems.length(); ++var)
    {
        int sizeDiff = fountSize * lrcItems[var].fontSize / 15;
        //判断这一行是否被选中
        if(var == selectLine)
        {
            //当前行被选中
            font.setPixelSize(fountSize + sizeDiff);
            painter.setFont(font);
            QStringList sl = lrcItems[var].line.toStringList(false);
            bool fast = true;
            QFontMetrics fm(font);
            for (auto& itm : sl)
            {
                //计算这一行宽度，计算横坐标
                pos.setY(pos.y() + fm.height());
                if(pos.y() > 0 && pos.y() < h + fountSize + sizeDiff && itm.length() > 0)
                {
                    pos.setX((w - fm.horizontalAdvance(itm)) / 2);
                    painter.setPen(Qt::red);
                    painter.drawText(pos, itm);
                }
                if(fast && selectWord >= 0 && wordLength > 0 && startTime > 0 && endTime > 0)
                {
                    fast = false;
                    //进度
                    int n_w = fm.horizontalAdvance(itm.mid(0, selectWord));
                    int n_w2 = fm.horizontalAdvance(itm.mid(selectWord, wordLength));
                    qint64 t1 = time - startTime;
                    qint64 t2 = endTime - startTime;
                    n_w = n_w + n_w2 * t1 / t2;
                    if(n_w > 0 && fm.height() > 0)
                    {
                        QPixmap n_image(n_w, fm.height());
                        n_image.fill();
                        QPainter n_painter(&n_image);
                        n_painter.setFont(font);
                        n_painter.setPen(Qt::green);
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
            QStringList sl = lrcItems[var].line.toStringList(false);
            for (auto& itm : sl)
            {
                //计算这一行宽度，计算横坐标
                pos.setY(pos.y() + fm.height());
                if(pos.y() > 0 && pos.y() < h + fm.height()  && itm.length() > 0)
                {
                    QFontMetrics fm(font);
                    pos.setX((w - fm.horizontalAdvance(itm)) / 2);
                    painter.setPen(Qt::blue);
                    painter.drawText(pos, itm);
                }
            }
        }
        pos.setY(pos.y() + fountSize / 2);
    }
    label->setPixmap(image);
    locker.unlock();
}

qint64 GLrc::setDispaleTime(qint64 time)
{
    QMutexLocker locker(updateMutex);
    qint64 t = lrcDispaleTime;
    lrcDispaleTime = time;
    locker.unlock();
    return t;
}

void GLrc::disableMovingPicture()
{
    QMutexLocker locker(updateMutex);
    m_disableMovingPicture = true;
    locker.unlock();
}

