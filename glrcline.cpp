#include "glrcline.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QCoreApplication>

GLrcLine::GLrcLine(QObject *parent)
    : QObject{parent}
{
    //加载模板
    QFile file(QCoreApplication::applicationDirPath() + "/nextChar.json");
    if(file.open(QFile::ReadOnly))
    {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        nextChar = doc.array();
    }

}

GLrcLine::GLrcLine(const GLrcLine &D)
: QObject{nullptr}
{
    nextChar = D.nextChar;
    lineItems = D.lineItems;
    lines = D.lines;
    select = D.select;
}

GLrcLine GLrcLine::operator=(const QString &str)
{
    //收到字符串，检查是否存在lrc标签
    QList<lrcLineItem> items;
    QStringList sl = str.split("\n");
    if(sl.size() == 0)
    {
        lineItems.clear();
        lines.clear();
        return *this;
    }
    items = getItems(sl[0]);
    if(items.length() == 1 && items[0].time == -1)
    {
        //进行第二套方案，拆分字符集
        items = splitWord(sl[0]);
    }
    lineItems = items;
    lines.clear();
    for (int var = 1; var < sl.length(); ++var)
    {
        append(sl[var]);
    }
    return *this;
}

bool GLrcLine::operator==(const GLrcLine &D)
{
    if(lineItems.length() != D.lineItems.length())
    {
        return false;
    }
    if(lines != D.lines)
    {
        return false;
    }
    for (int var = 0; var < lineItems.length(); ++var)
    {
        if(lineItems[var].time != D.lineItems[var].time)
        {
            return false;
        }
        if(lineItems[var].word != D.lineItems[var].word)
        {
            return false;
        }
    }
    return true;
}

GLrcLine GLrcLine::operator=(const GLrcLine &D)
{
    lineItems = D.lineItems;
    lines = D.lines;
    select = D.select;
    return *this;
}

QString GLrcLine::toString(bool incuudeTimes)
{
    QString retStr;
    for (const auto& itm:lineItems)
    {
        if(incuudeTimes && itm.time != -1)
        {
            QTime t = QTime::fromMSecsSinceStartOfDay(itm.time);
            QString ts = t.toString("mm:ss.") + t.toString("zzz").left(2);
            retStr += "<" + ts + ">";
        }
        retStr += itm.word;
    }
    for (int var = 0; var < lines.size(); ++var)
    {
        retStr += "\n" + lines[var];
    }
    return retStr;
}

QStringList GLrcLine::toStringList(bool incuudeTimes)
{
    QString retStr;
    for (const auto& itm:lineItems)
    {
        if(incuudeTimes && itm.time != -1)
        {
            QTime t = QTime::fromMSecsSinceStartOfDay(itm.time);
            QString ts = t.toString("mm:ss.") + t.toString("zzz").left(2);
            retStr += "<" + ts + ">";
        }
        retStr += itm.word;
    }
    QStringList retList;
    retList.append(retStr);
    retList.append(lines);
    return retList;
}

QString GLrcLine::toHtml(bool sel, int lfCount)
{
    QString html;
    QString word2[3] = {"w1", "w2", "w3"};
    for (int var = 0; var < lineItems.length(); ++var)
    {
        QString lWord = lineItems[var].word;
        if(lWord == "")
            lWord = " ";
        if(sel)
        {
            if(var == select)
            {
                html += R"(<span class="wordS word )" + word2[var%3] + R"(">)" + lWord.toHtmlEscaped() + "</span>";
            }
            else if(var < select)
            {
                html += R"(<span class="wordL word )" + word2[var%3] + R"(">)" + lWord.toHtmlEscaped() + "</span>";
            }
            else if(var > select)
            {
                html += R"(<span class="wordR word )" + word2[var%3] + R"(">)" + lWord.toHtmlEscaped() + "</span>";
            }
        }
        else
        {
            html += R"(<span class="word">)" + lWord.toHtmlEscaped() + "</span>";
        }
    }
    for (int var = 0; var < lines.size() || var < lfCount; ++var)
    {
        if(var < lines.size())
        {
            html += "\n" + lines[var].toHtmlEscaped();
        }
        else
        {
            html += "\n";
        }
    }
    return html;
}

int GLrcLine::selectTime(qint64 time)
{
    int ls = -1;
    for (int var = 0; var < lineItems.length(); ++var)
    {
        if(lineItems[var].time <= time && lineItems[var].time != -1)
        {
            ls = var;
        }
    }
    select = ls;
    return ls;
}

qint64 GLrcLine::selectId(int id)
{
    if(id >= lineItems.length())
    {
        id = lineItems.length() - 1;
    }
    if(id < -1)
    {
        id = -1;
    }
    select = id;
    if(id == -1)
    {
        return -1;
    }
    return lineItems[id].time;
}

int GLrcLine::selectNext()
{
    if(select < lineItems.length() - 1)
    {
        select++;
    }
    return select;
}

int GLrcLine::selectPrevious()
{
    if(select > 0)
    {
        select--;
    }
    return select;
}

int GLrcLine::getSelect()
{
    return select;
}

bool GLrcLine::isSpace()
{
    if(lineItems.size() != 0)
    {
        if(lineItems[0].word != "" || lineItems[0].time != -1)
        {
            return false;
        }
    }
    if(lines.size() != 0)
    {
        return false;
    }
    return true;
}

void GLrcLine::append(QString line)
{
    lines.append(line);
}

qint64 GLrcLine::setTime(qint64 time)
{
    if(select >= 0 && select < lineItems.length())
    {
        qint64 t = lineItems[select].time;
        lineItems[select].time = time;
        return t;
    }
    return -1;
}

qint64 GLrcLine::getTime(int _id) const
{
    if(_id < 0)
    {
        _id = select;
    }
    if(_id >= 0 && _id < lineItems.length())
    {
        return lineItems[_id].time;
    }
    return -1;
}

int GLrcLine::deleteAllTime()
{
    int rev = 0;
    for (int var = 0; var < lineItems.length(); ++var)
    {
        if(lineItems[var].time != -1)
            rev++;
        lineItems[var].time = -1;
    }
    return rev;
}

qint64 GLrcLine::deleteTime()
{
    qint64 t = -1;
    if(select >= 0 && select < lineItems.length())
    {
        t = lineItems[select].time;
        lineItems[select].time = -1;
    }
    return t;
}

int GLrcLine::status(qint64 time, qint64 *selectTime, qint64 *nextTime, int *seleteWorld, int* seleteWorldLength)
{
    *seleteWorld = 0;
    *seleteWorldLength = 0;
    *selectTime = -1;
    *nextTime = -1;
    for (int var = 0; var < lineItems.length(); ++var)
    {
        if(lineItems[var].time == -1)
        {
            break;
        }
        if(lineItems[var].time <= time)
        {
            *selectTime = lineItems[var].time;
            *seleteWorld += *seleteWorldLength;
            *seleteWorldLength = lineItems[var].word.length();
        }
        else
        {
            *nextTime = lineItems[var].time;
            break;
        }
    }
    return lineItems.length();
}

int GLrcLine::getLineSum()
{
    return lines.length() + 1;
}


bool GLrcLine::continuous(QChar ch, QChar next)
{
    for (const auto& itm : qAsConst(nextChar))
    {
        QJsonObject obj = itm.toObject();
        QJsonArray arr = obj.find("char")->toArray();
        for(const auto& itm : qAsConst(arr))
        {
            QJsonObject c = itm.toObject();
            int s = 0;
            if(c.find("start") != c.end())
                s = c.find("start")->toInt();
            int m = 0;
            if(c.find("end") != c.end())
                m = c.find("end")->toInt();
            QString cs = "";
            if(c.find("chars") != c.end())
                cs = c.find("chars")->toString();
            if((ch.unicode() >= s && ch.unicode() <= m) || cs.indexOf(ch) != -1)
            {
                QJsonArray arr = obj.find("next")->toArray();
                for(const auto& itm : qAsConst(arr))
                {
                    QJsonObject c = itm.toObject();
                    int s = 0;
                    if(c.find("start") != c.end())
                        s = c.find("start")->toInt();
                    int m = 0;
                    if(c.find("end") != c.end())
                        m = c.find("end")->toInt();
                    QString cs = "";
                    if(c.find("chars") != c.end())
                        cs = c.find("chars")->toString();
                    if((next.unicode() >= s && next.unicode() <= m) || cs.indexOf(next) != -1)
                    {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

qint64 GLrcLine::getTime(QString str, int start)
{
    if(str.mid(start, 1) != "<")
    {
        return -1;
    }
    QString timeStr = str.mid(start + 1,str.indexOf(">") - 1);
    QTime time = QTime::fromString(timeStr + "0","mm:ss.zzz");
    if(time.isNull())
    {
        time = QTime::fromString(timeStr,"mm:ss.zzz");
    }
    if(time.isNull())
    {
        return -1;
    }
    return time.msecsSinceStartOfDay();
}

QList<lrcLineItem> GLrcLine::getItems(QString line)
{
    QList<lrcLineItem> items;
    int tagStart = 0;
    qint64 timeStart = getTime(line, 0);
    if(timeStart != -1)
    {
        tagStart = 10;
    }
    for (int var = tagStart; var < line.length(); ++var)
    {
        qint64 ltime = getTime(line, var);
        if(ltime != -1)
        {
            //找到下一个标签
            lrcLineItem litem;
            litem.time = timeStart;
            litem.word = line.mid(tagStart, var - tagStart);
            items.append(litem);
            tagStart = var + 10;
            var += 9;
            timeStart = ltime;
        }
    }
    //结尾，最后一个项目
    lrcLineItem litem;
    litem.time = timeStart;
    litem.word = line.mid(tagStart);
    items.append(litem);
    return items;
}

QList<lrcLineItem> GLrcLine::splitWord(QString line)
{
    QList<lrcLineItem> items;
    int start = 0;
    for (int var = 0; var < line.length() - 1; ++var)
    {
        //检查下一字符是否连续
        if(!continuous(line[var], line[var + 1]))
        {
            //不是连续字符
            lrcLineItem litem;
            litem.time = -1;
            litem.word = line.mid(start, var - start + 1);
            start = var + 1;
            items.append(litem);
        }
    }
    //检查最后一个
    lrcLineItem litem;
    litem.time = -1;
    litem.word = line.mid(start);
    items.append(litem);
    return items;
}
