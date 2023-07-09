#ifndef GLRC_H
#define GLRC_H

#include <QObject>
#include <QList>
#include <glrcline.h>

class QThread;
class QLabel;
class QMutex;

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
    explicit GLrc(QObject *parent = nullptr);
    ~GLrc();
    void setLrc(QString lrc, int maxLine = 2147483647);
    QString getLrc(bool moreTime = false);
    QString getHtml(qint64 time = -1, bool includTimes = true,qint64* line = nullptr, int* pos = nullptr, bool wordColor = false);
    QString getHtmlFx(qint64 time = -1, bool includTimes = true,qint64* line = nullptr, int* pos = nullptr, bool wordColor = false);
    QString getHtmlFx2(qint64 time = -1, bool includTimes = true,qint64* line = nullptr, int* pos = nullptr, bool wordColor = false);
    qint64 previousItem();
    qint64 previousWord();
    qint64 nextItem();
    qint64 nextLine();
    qint64 nextWord();
    int getSelectLine();
    qint64 getLrcTime(qint64 time);
    qint64 getSelectTime();
    QString getLine(int line);
    QString getTimes(int line);
    bool setLine(int line, QString text);
    bool setTimes(int line, QString times);
    qint64 timeAdd(qint64 offset);
    qint64 wordTimeAdd(qint64 offset);
    bool instTime(qint64 time, int line = -1);
    qint64 removeTime(int line = -1, int itm = -1);
    void mergeDuplicates();
    int instLine(int line = -1);
    void lrcTimesSort();
    int size();
    void setSelectLine(int line);
    bool removeLine(int line = -1);
    qint64 setWordTime(qint64 time);
    qint64 getWordTime();
    int getSelectWord();
    qint64 selectWordId(int id);
    qint64 deleteWordTime();
    int deleteLineWordTime();
    int deleteAllWordTime();

    QLabel * setLabel(QLabel* _label);
    void updateLrcwindow(qint64 time);
    qint64 setDispaleTime(qint64 time);
    void disableMovingPicture();

signals:
    void lrcChanged();
private:
    QList<lrcItem> lrcItems;
    qint64 getTimeOfLrcLine(QString& lrcLine);
    qint64 getSmallTime(lrcItem line);
    int selectTime = 0;
    int selectLine = 0;
    void prLrcSort();
    QThread* lrcThread = nullptr;
    bool threadRunning;
    qint64 lrcDispaleTime = 0;
    static void lrcDispaleThread(GLrc* lrc);
    QLabel* label = nullptr;
    void status(qint64 time,int *line, int *word,int* wordSeleteLength, int *wordSize, qint64 *startTime, qint64 *endTime);
    int getTextSize(int w, int h);
    QMutex * updateMutex = nullptr;
    int movSpeed(int length);
    bool m_disableMovingPicture = true;

};

#endif // GLRC_H
