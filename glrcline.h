#ifndef GLRCLINE_H
#define GLRCLINE_H

#include <QObject>
#include <QList>
#include <QJsonArray>

struct lrcLineItem
{
    qint64 time;
    QString word;
};

class GLrcLine : public QObject
{
    Q_OBJECT
public:
    explicit GLrcLine(QObject *parent = nullptr);
    GLrcLine(const GLrcLine &D);
    GLrcLine operator=(const QString &D);
    GLrcLine operator=(const GLrcLine &D);
    bool operator==(const GLrcLine &D);
    QString toString(bool incuudeTimes = true);
    QStringList toStringList(bool incuudeTimes = true);
    QString toHtml(bool sel = false, int lfCount = 0);
    QString toSrt(int sel = -1);
    int selectTime(qint64 time);
    qint64 selectId(int id);
    int selectNext();
    int selectPrevious();
    int getSelect();
    bool isSpace();
    void append(QString line);
    qint64 setTime(qint64 time);
    qint64 getTime(int _id = -1) const;
    int deleteAllTime();
    qint64 deleteTime();
    int status(qint64 time, qint64* selectTime, qint64* nextTime, int* seleteWorld, int* seleteWorldLength);
    int getLineSum();
    int itmCount();

signals:

private:
    QList<lrcLineItem> lineItems;
    QList<QString> lines;
    //单字符false，连续字符返回true
    bool continuous(QChar ch, QChar next);
    QJsonArray nextChar;
    qint64 getTime(QString str, int start);
    QList<lrcLineItem> getItems(QString line);
    QList<lrcLineItem> splitWord(QString line);
    int select = -1;
};

#endif // GLRCLINE_H
