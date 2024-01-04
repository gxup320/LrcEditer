#ifndef LINEEDIT_H
#define LINEEDIT_H

#include <QDialog>

namespace Ui {
class LineEdit;
}

class MainWindow;

class LineEdit : public QDialog
{
    Q_OBJECT

public:
    explicit LineEdit(QWidget *parent = nullptr);
    ~LineEdit();
    MainWindow* m;
    int exec();

private slots:
    void on_pushButton_save_clicked();

    void on_pushButton_close_clicked();

private:
    Ui::LineEdit *ui;
    int selectLine;
};

#endif // LINEEDIT_H
