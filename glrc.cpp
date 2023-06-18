#include "glrc.h"
#include <QTime>
#include <QStringList>

GLrc::GLrc(QObject *parent)
    : QObject{parent}
{

}

void GLrc::setLrc(QString lrc, int maxLine)
{
    if(maxLine < 1)
        maxLine = 2147483647;
    lrcItems.resize(0);
    lrc.replace("\r\n", "\n");
    lrc.replace("\r", "\n");
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
    qint64 localTime = -1;
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
    for(auto& itm:lrcItems)
    {
        for(auto i:itm.times)
        {
            if(i > localTime && i <= time)
                localTime = i;
        }
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
    lrcItems[selectLine].line.selectNext();
    qint64 time = lrcItems[selectLine].line.getTime();
    //emit lrcChanged();
    return time;
}

int GLrc::getSelectLine()
{
    return selectLine;
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
    if(line >= lrcItems.size())
        return false;
    lrcItems[line].line = text;
    emit lrcChanged();
    return true;
}

bool GLrc::setTimes(int line, QString times)
{
    if(line >= lrcItems.size())
        return false;
    qint64 time = getTimeOfLrcLine(times);
    QList<qint64> arr;
    while(time != -1)
    {
        arr << time;
        time = getTimeOfLrcLine(times);
    }
    lrcItems[line].times = arr;
    emit lrcChanged();
    return true;
}

qint64 GLrc::timeAdd(qint64 offset)
{
    if(selectLine >= lrcItems.size())
        return 0;
    qint64 localTime = lrcItems[selectLine].times[selectTime] + offset;
    if(localTime < 0)
    {
        localTime = 0;
    }
    lrcItems[selectLine].times[selectTime] = localTime;
    emit lrcChanged();
    return localTime;
}

qint64 GLrc::wordTimeAdd(qint64 offset)
{
    if(selectLine >= lrcItems.size())
        return 0;
    qint64 localTime = lrcItems[selectLine].line.getTime() + offset;
    if(localTime < lrcItems[selectLine].times[selectTime])
    {
        localTime = lrcItems[selectLine].times[selectTime];
    }
    lrcItems[selectLine].line.setTime(localTime);
    emit lrcChanged();
    return localTime;
}

bool GLrc::instTime(qint64 time, int line)
{
    if(lrcItems.size() == 0)
        return false;
    if(line == -1)
        line = selectLine;
    if(line >= lrcItems.size())
        return false;
    lrcItems[line].times << time;
    emit lrcChanged();
    return true;
}

qint64 GLrc::removeTime(int line, int itm)
{
    if(line == -1)
        line = selectLine;
    if(itm == -1)
    {
        itm = selectTime;
        if(selectTime > 0)
            selectTime--;
    }
    if(line >= lrcItems.size())
        return -1;
    if(itm >= lrcItems[line].times.size())
        itm = lrcItems[line].times.size() - 1;
    if(itm < 0)
        return -1;
    qint64 time = lrcItems[line].times[itm];
    lrcItems[line].times.remove(itm);
    emit lrcChanged();
    return time;
}

void GLrc::mergeDuplicates()
{
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
    emit lrcChanged();
}

int GLrc::instLine(int line)
{
    lrcItem t;
    if(line == -1)
    {
        line = selectLine;
    }
    if(line > lrcItems.size())
        line = lrcItems.size();
    lrcItems.insert(line,t);
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

void GLrc::lrcTimesSort()
{
    prLrcSort();
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
    emit lrcChanged();
    return true;
}

qint64 GLrc::setWordTime(qint64 time)
{
    if(selectLine >= 0 && selectLine < lrcItems.length())
    {
        qint64 t = lrcItems[selectLine].line.setTime(time);
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
    qint64 t = lrcItems[selectLine].line.deleteTime();
    if(t != -1)
    {
        emit lrcChanged();
    }
    return t;
}

int GLrc::deleteLineWordTime()
{
    int rev = lrcItems[selectLine].line.deleteAllTime();
    if(rev != 0)
    {
        emit lrcChanged();
    }
    return rev;
}

int GLrc::deleteAllWordTime()
{
    int rev = 0;
    for (int var = 0; var < lrcItems.length(); ++var)
    {
        rev += lrcItems[var].line.deleteAllTime();
    }
    if(rev != 0)
    {
        emit lrcChanged();
    }
    return rev;
}

