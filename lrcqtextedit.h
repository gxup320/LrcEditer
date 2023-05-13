#ifndef LRCQTEXTEDIT_H
#define LRCQTEXTEDIT_H

#include <QTextEdit>
#include <QObject>
#include <QWidget>

class lrcQTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    lrcQTextEdit(QWidget *parent = nullptr);
    bool disableKeyEvent = false;
private:
    void keyPressEvent(QKeyEvent *event);
    void focusOutEvent(QFocusEvent *e);

private slots:

signals:
    bool keyProc(QKeyEvent *event);
    void focusOutProc(QFocusEvent *event);
};

#endif // LRCQTEXTEDIT_H
