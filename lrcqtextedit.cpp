#include "lrcqtextedit.h"

lrcQTextEdit::lrcQTextEdit(QWidget *parent)
    :QTextEdit(parent)
{

}

void lrcQTextEdit::keyPressEvent(QKeyEvent *event)
{
    if(emit keyProc(event))
        QTextEdit::keyPressEvent(event);
}

void lrcQTextEdit::focusOutEvent(QFocusEvent *event)
{
    emit focusOutProc(event);
}
