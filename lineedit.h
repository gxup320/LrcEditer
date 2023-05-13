#ifndef LINEEDIT_H
#define LINEEDIT_H

#include <QWidget>

namespace Ui {
class LineEdit;
}

class MainWindow;

class LineEdit : public QWidget
{
    Q_OBJECT

public:
    explicit LineEdit(QWidget *parent = nullptr);
    ~LineEdit();
    MainWindow* m;
    void show();
    bool close();

private slots:
    void on_pushButton_save_clicked();

    void on_pushButton_close_clicked();

private:
    Ui::LineEdit *ui;
    int selectLine;
    void closeEvent( QCloseEvent * event );
};

#endif // LINEEDIT_H
