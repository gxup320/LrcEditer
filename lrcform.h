#ifndef LRCFORM_H
#define LRCFORM_H

#include <QWidget>

namespace Ui {
class lrcForm;
}
class MainWindow;
class QLabel;

class lrcForm : public QWidget
{
    Q_OBJECT

public:
    explicit lrcForm(QWidget *parent = nullptr);
    ~lrcForm();
    MainWindow* m;
    void show();
    void showFullScreen();
    QSize m_labelSize;
    QLabel* m_label;

private:
    Ui::lrcForm *ui;
    void closeEvent( QCloseEvent * event );
    void resizeEvent(QResizeEvent *event);
};

#endif // LRCFORM_H
