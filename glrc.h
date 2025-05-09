#ifndef GLRC_H
#define GLRC_H

#include <QObject>
#include <QList>
#include <QSize>
#include <glrcline.h>

class GLrc : public QObject
{
    Q_OBJECT
public:
    struct lrcItem
    {
        QList<qint64> times;
        GLrcLine line;
        int fontSize = -5;
    };
    enum mode{AUTO,MERGE,SPLIT};
    explicit GLrc(QObject *parent = nullptr);
    GLrc(GLrc & cp);
    ~GLrc();
    void setLrc(QString lrc, int maxLine = 2147483647);
    QString getLrc(bool moreTime = false, bool syncLineFistTime = true);
    QString getHtml(qint64 time = -1, bool includTimes = true,qint64* line = nullptr, int* pos = nullptr, bool wordColor = false);
    QString getHtmlFx(qint64 time = -1, bool includTimes = true,qint64* line = nullptr, int* pos = nullptr, bool wordColor = false);
    QString getHtmlFx2(qint64 time = -1, bool includTimes = true,qint64* line = nullptr, int* pos = nullptr, bool wordColor = false);
    QString toSrt(qint64 maximumTime = 30000);
    qint64 previousItem();
    qint64 previousWord();
    qint64 nextItem();
    qint64 nextLine();
    qint64 nextWord();
    int getSelectLine();
    qint64 getLrcTime(qint64 time);
    qint64 getLrcNextTime(qint64 time);
    qint64 getSelectTime();
    QString getLineString(int line,bool incuudeTimes = true);
    QList<qint64> getLineTime(int line);
    QString getTimes(int line);
    bool setLine(int line, QString text);
    bool setTimes(int line, QString times);
    qint64 timeAdd(qint64 offset);
    void timeAddAll(qint64 offset);
    qint64 wordTimeAdd(qint64 offset);
    bool instTime(qint64 time, int line = -1);
    qint64 removeTime(int line = -1, int itm = -1);
    void mergeDuplicates(mode m = AUTO);
    int instLine(int line = -1);
    void lrcTimesSort();
    int size();
    void setSelectLine(int line);
    void setSeleteTime(qint64 time);
    bool removeLine(int line = -1);
    qint64 setWordTime(qint64 time);
    qint64 getWordTime();
    int getSelectWord();
    qint64 selectWordId(int id);
    qint64 deleteWordTime();
    int deleteLineWordTime();
    int deleteAllWordTime();
    int replaceTime(qint64 sorce, qint64 target);
    QList<lrcItem> getAllItems();
    int getTextSize(int w, int h);
    void status(qint64 time,QList<int> *line, QList<int> *word,QList<int>* wordSeleteLength, QList<int> *wordSize, QList<qint64> *startTime, QList<qint64> *endTime);

public slots:

signals:
    void lrcChanged(int = 0);

private:
    QList<lrcItem> lrcItems;
    qint64 getTimeOfLrcLine(QString& lrcLine);
    qint64 getSmallTime(lrcItem line);
    int selectTime = 0;
    int selectLine = 0;
    void prLrcSort(QList<lrcItem>& items);
};

#endif // GLRC_H
