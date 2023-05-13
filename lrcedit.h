#ifndef LRCEDIT_H
#define LRCEDIT_H

#include <QWidget>

namespace Ui {
class LrcEdit;
}

class MainWindow;

class LrcEdit : public QWidget
{
    Q_OBJECT

public:
    explicit LrcEdit(QWidget *parent = nullptr);
    ~LrcEdit();
    MainWindow* m;
    void show();
    bool close();

private slots:
    void on_pushButton_save_clicked();

    void on_pushButton_close_clicked();

private:
    Ui::LrcEdit *ui;
    void closeEvent( QCloseEvent * event );
};

#endif // LRCEDIT_H
