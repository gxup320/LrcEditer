#ifndef BUFFERSIZEEDIT_H
#define BUFFERSIZEEDIT_H

#include <QDialog>
class QAbstractButton;

class MainWindow;

namespace Ui {
class BufferSizeEdit;
}

class BufferSizeEdit : public QDialog
{
    Q_OBJECT

public:
    explicit BufferSizeEdit(QWidget *parent = nullptr);
    ~BufferSizeEdit();
    int exec();
    MainWindow* m;

private slots:
    void on_BufferSizeEdit_accepted();

private:
    Ui::BufferSizeEdit *ui;
};

#endif // BUFFERSIZEEDIT_H
