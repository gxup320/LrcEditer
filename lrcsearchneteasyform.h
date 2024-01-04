#ifndef LRCSEARCHNETEASYFORM_H
#define LRCSEARCHNETEASYFORM_H

#include <QDialog>

namespace Ui {
class LrcSearchNeteasyForm;
}

class MainWindow;

struct lrcTime
{
    qint64 time;
    int line;
};

class LrcSearchNeteasyForm : public QDialog
{
    Q_OBJECT

public:
    explicit LrcSearchNeteasyForm(QWidget *parent = nullptr);
    ~LrcSearchNeteasyForm();
    MainWindow* m;
    int exec();
    bool close();

private slots:
    void on_comboBox_currentIndexChanged(int index);

    void on_pushButton_search_clicked();

    void on_pushButton_close_clicked();

    void on_pushButton_apply_clicked();

    void on_lineEdit_url_textChanged(const QString &arg1);

private:
    Ui::LrcSearchNeteasyForm *ui;
    bool wangyiyunList(QString name);
    QJsonArray* musicArr;
    void lrcSort(QString &fullLrc);
    qint64 getTimeOfLrcLine(QString &lrcLine);
    void lrcTimesSort(QList<lrcTime> &lrcTimes);
    void closeEvent( QCloseEvent * event );
};

#endif // LRCSEARCHNETEASYFORM_H
