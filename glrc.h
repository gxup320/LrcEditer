#ifndef GLRC_H
#define GLRC_H

#include <QObject>
#include <QList>
#include <QSize>
#include <glrcline.h>

class QThread;
//class QMutex;

struct lrcItem
{
    QList<qint64> times;
    GLrcLine line;
    int fontSize = -5;
};

class GLrc : public QObject
{
    Q_OBJECT
public:
    enum mode{AUTO,MERGE,SPLIT};
    explicit GLrc(QObject *parent = nullptr);
    GLrc(GLrc & cp);
    ~GLrc();
    void setLrc(QString lrc, int maxLine = 2147483647);
    QString getLrc(bool moreTime = false);
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

    void setDispaleColor(const QColor& _default,const QColor& _selectLine,const QColor& _selectLineOver,const QColor& _selectWord);
public slots:
    QSize setLabelSize(QSize _labelSize);
    void updateLrcwindow(qint64 time);
    qint64 setDispaleTime(qint64 time);
    void disableMovingPicture();
    const QPixmap* getPixmap();

signals:
    void lrcChanged(int = 0);
    void lrcImgChanged();

private:
    QList<lrcItem> lrcItems;
    qint64 getTimeOfLrcLine(QString& lrcLine);
    qint64 getSmallTime(lrcItem line);
    int selectTime = 0;
    int selectLine = 0;
    void prLrcSort(QList<lrcItem>& items);
    QThread* lrcThread = nullptr;
    bool threadRunning;
    qint64 lrcDispaleTime = 0;
    static void lrcDispaleThread(GLrc* lrc);
    QSize labelSize = {0 , 0};
    void status(QList<lrcItem>& c_lrcItems, qint64 time,QList<int> *line, QList<int> *word,QList<int>* wordSeleteLength, QList<int> *wordSize, QList<qint64> *startTime, QList<qint64> *endTime);
    int getTextSize(QList<lrcItem>& c_lrcItems, int w, int h);
    //QMutex * updateMutex = nullptr;
    //QMutex * lrcDispale = nullptr;
    int movSpeed(int length);
    bool m_disableMovingPicture = true;
    QPixmap* image = nullptr;
    QPixmap* imageNext = nullptr;
    QPixmap* imageGet = nullptr;
    bool imgReadEd = true;
    QColor* colors;
};

#endif // GLRC_H
