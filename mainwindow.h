#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>
#include "gaudioplayer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class LineEdit;
class LrcEdit;
class LrcSearchNeteasyForm;
class batchProcessing;
class BufferSizeEdit;
class lrcForm;

class GLrc;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    GLrc* lrc;
    void displayLrc(qint64 time, bool f = false);
    QString netEasyApiUrl;
    void loadLrc(QString fileName);
    qint64 getBufferSize();
    qint64 getBufferSizeSmall();
    qint64 setBufferSize(qint64 size, qint64 sizeSmall);

private slots:
    void durationChanged(qint64 duration);
    void positionChanged(qint64 position);
    void metaDataChanged(QMediaMetaData mediaData);
    void valFromMouse(int val);
    void loadStatus(qint64 position, bool isEnd);
    void lrcChanged();
    void bufferSizeChanged(qint64 size);

    void on_pushButton_loadmusic_clicked();
    void on_pushButton_tostart_clicked();

    void on_pushButton_play_pause_clicked();

    void on_pushButton_loadlrc_clicked();

    void on_pushButton_save_clicked();

    void on_pushButton_save_as_clicked();

    void on_pushButton_inst_clicked();

    void on_pushButton_remove_clicked();

    bool keyProc(QKeyEvent *event);

    void focusOutProc(QFocusEvent *event);

    void on_textEdit_textChanged();

    void on_pushButton_sort_clicked();

    void on_pushButton_loadlrc_neteasy_clicked();

    void on_horizontalSlider_valueChanged(int value);




    void on_pushButton_merge_duplicates_clicked();

    void on_pushButton_edit_line_clicked();

    void on_pushButton_edit_lrc_clicked();

    void on_pushButton_inst_line_clicked();

    void on_pushButton_append_line_clicked();

    void on_pushButton_search_from_neteasy_clicked();

    void on_pushButton_remove_line_clicked();

    void on_checkBox_accurate_process_stateChanged(int arg1);

    void on_pushButto_batchProcess_clicked();

    void on_checkBox_debug_stateChanged(int arg1);

    void on_pushButton_deleteWordTime_clicked();

    void on_pushButton_deleteLineWordTime_clicked();

    void on_pushButton_deleteAllWordTime_clicked();

    void on_pushButton_bufferSize_clicked();

    void on_pushButton_showLrcWindow_clicked();

    void on_pushButtonfullScreen_clicked();

private:
    Ui::MainWindow *ui;
    LineEdit* linee = nullptr;
    LrcEdit* lrce = nullptr;
    LrcSearchNeteasyForm* m_LrcSearchNeteasyForm = nullptr;
    batchProcessing* m_batchProcessing = nullptr;
    BufferSizeEdit* m_BufferSizeEdit = nullptr;
    lrcForm* m_lrcForm = nullptr;
    //QMediaPlayer* mediaPlayer = nullptr;
    //QAudioOutput* audioOutput = nullptr;
    //QVideoWidget* videoOutput = nullptr;
    GAudioPlayer * player = nullptr;
    QString lrcFileName;
    //qint64 getTimeOfLrcLine(QString& lrcLine);
    //void lrcTimesSort(QList<lrcTime>& lrcTimes);
    //void setSelectLrcLine(int lrcLine);
    //void setSelectLrcLineShowLine(int lrcLine, int showN = 5);
    //int getSelectLrcLine();
    //void instLrcTime(int lrcLine, qint64 time = -1);
    //qint64 removeLrcTime(int lrcLine);
    //qint64 getLrcTimeLine(int lrcLine);
    bool saveLrcToFile(QString fileName);
    void closeEvent( QCloseEvent * event );
    friend class LrcSearchNeteasyForm;
};
#endif // MAINWINDOW_H
