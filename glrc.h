#ifndef GLRC_H
#define GLRC_H

#include <QObject>
#include <QList>

struct lrcItem
{
    QList<qint64> times;
    QString line;
};

class GLrc : public QObject
{
    Q_OBJECT
public:
    explicit GLrc(QObject *parent = nullptr);
    void setLrc(QString lrc, int maxLine = 2147483647);
    QString getLrc(bool moreTime = false);
    QString getHtml(qint64 time = -1, bool includTimes = true,qint64* line = nullptr, int* pos = nullptr);
    qint64 previousItem();
    qint64 nextItem();
    qint64 nextLine();
    int getSelectLine();
    QString getLine(int line);
    QString getTimes(int line);
    bool setLine(int line, QString text);
    bool setTimes(int line, QString times);
    qint64 timeAdd(qint64 offset);
    bool instTime(qint64 time, int line = -1);
    qint64 removeTime(int line = -1, int itm = -1);
    void mergeDuplicates();
    int instLine(int line = -1);
    void lrcTimesSort();
    int size();
    void setSelectLine(int line);
    bool removeLine(int line = -1);

signals:
    void lrcChanged();
private:
    QList<lrcItem> lrcItems;
    qint64 getTimeOfLrcLine(QString& lrcLine);
    qint64 getSmallTime(lrcItem line);
    int selectTime = 0;
    int selectLine = 0;
    void prLrcSort();

};

#endif // GLRC_H
