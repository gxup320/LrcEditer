#include "lrcform.h"
#include "mainwindow.h"
#include "glrc.h"
#include "ui_lrcform.h"
#include <QLabel>
#include <QSizePolicy>

lrcForm::lrcForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::lrcForm)
{
    ui->setupUi(this);
}

lrcForm::~lrcForm()
{
    delete ui;
}

void lrcForm::show()
{
    ui->label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    m_label = m->lrc->setLabel(ui->label);
    m->lrc->disableMovingPicture();
    QWidget::show();
}

void lrcForm::closeEvent(QCloseEvent *)
{
    m->lrc->setLabel(m_label);
    m->lrc->disableMovingPicture();
    m_label->show();
}

void lrcForm::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    m->lrc->disableMovingPicture();
}
