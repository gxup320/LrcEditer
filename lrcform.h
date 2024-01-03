#ifndef LRCFORM_H
#define LRCFORM_H

#include <QWidget>
class GLrcWindowGL;

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
    GLrcWindowGL* lrcWindow;

private:
    Ui::lrcForm *ui;
    void mouseDoubleClickEvent(QMouseEvent *event);
};

#endif // LRCFORM_H
