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
    QString toString();
    QStringList toStringList();
    QString toHtml(bool sel = false, int lfCount = 0);
    int selectTime(qint64 time);
    qint64 selectId(int id);
    int selectNext();
    int selectPrevious();
    int getSelect();
    bool isSpace();
    void append(QString line);
    qint64 setTime(qint64 time);
    qint64 getTime() const;
    int deleteAllTime();
    qint64 deleteTime();

signals:

private:
    QList<lrcLineItem> lineItems;
    QList<QString> lines;
    //全角返回0，半角返回1，标点符号返回2
    bool continuous(QChar ch, QChar next);
    QJsonArray nextChar;
    qint64 getTime(QString str, int start);
    QList<lrcLineItem> getItems(QString line);
    QList<lrcLineItem> splitWord(QString line);
    int select = -1;
};

#endif // GLRCLINE_H
