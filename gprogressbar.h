#ifndef GPROGRESSBAR_H
#define GPROGRESSBAR_H

#include <QProgressBar>
#include <QObject>

class GProgressBar : public QProgressBar
{
    Q_OBJECT
public:
    GProgressBar(QWidget *parent = nullptr);
signals:
    void valFromMouse(qint64 val);
private:
    void mousePressEvent(QMouseEvent *e);
};

#endif // GPROGRESSBAR_H
