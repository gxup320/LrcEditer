#ifndef BATCHPROCESSING_H
#define BATCHPROCESSING_H

#include <QDialog>

class MainWindow;
class QStandardItemModel;
class GLrc;

namespace Ui {
class batchProcessing;
}

class batchProcessing : public QDialog
{
    Q_OBJECT

public:
    explicit batchProcessing(QWidget *parent = nullptr);
    ~batchProcessing();
    void fileListAddItem(const QString & fileName);
    void loadLrc(QString fileName);
    bool saveLrcToFile(QString fileName);

private slots:
    void on_pushButton_add_file_clicked();

    void on_pushButton_add_directory_clicked();

    void on_pushButton_remove_clicked();

    void on_pushButton_outputSelect_clicked();

    void on_pushButton_execute_clicked();

private:
    GLrc *lrc;
    Ui::batchProcessing *ui;
    QStandardItemModel* model_files;

    QString al, ar, au, by, ti, re, ve;
    int offset = 0;
};

#endif // BATCHPROCESSING_H
