#ifndef LRCEDIT_H
#define LRCEDIT_H

#include <QDialog>

namespace Ui {
class LrcEdit;
}

class MainWindow;

class LrcEdit : public QDialog
{
    Q_OBJECT

public:
    explicit LrcEdit(QWidget *parent = nullptr);
    ~LrcEdit();
    MainWindow* m;
    int exec();

private slots:
    void on_pushButton_save_clicked();

    void on_pushButton_close_clicked();

private:
    Ui::LrcEdit *ui;
};

#endif // LRCEDIT_H
