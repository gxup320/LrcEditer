#include "lrcform.h"
#include "ui_lrcform.h"
#include <QLabel>
#include <QSizePolicy>

lrcForm::lrcForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::lrcForm)
{
    ui->setupUi(this);
    lrcWindow = ui->openGLWidget;
}

lrcForm::~lrcForm()
{
    delete ui;
}

void lrcForm::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(isFullScreen())
    {
        showNormal();
    }
    else
    {
        showFullScreen();
    }
}

