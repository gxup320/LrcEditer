#include "lrcform.h"
#include "mainwindow.h"
#include "glrc.h"
#include "ui_lrcform.h"
#include <QLabel>

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
    m_label = m->lrc->setLabel(ui->label);
    QWidget::show();
}

void lrcForm::closeEvent(QCloseEvent *event)
{
    m->lrc->setLabel(m_label);
    m_label->show();
}
