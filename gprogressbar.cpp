 #include "gprogressbar.h"
#include <QMouseEvent>

GProgressBar::GProgressBar(QWidget *parent)
    :QProgressBar(parent)
{

}

void GProgressBar::mousePressEvent(QMouseEvent *e)
{
    QProgressBar::mousePressEvent(e);
    double p = (double)(e->pos().x()) / (double)width();
    qint64 val = p*(maximum()-minimum())+minimum();
    emit valFromMouse(val);
}
