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
    m_labelSize = m->lrc->setLabelSize(ui->label->size());
    m_label = m->m_lrcLabel;
    m->m_lrcLabel = ui->label;
    QWidget::show();
}

void lrcForm::showFullScreen()
{
    m_labelSize = m->lrc->setLabelSize(ui->label->size());
    m_label = m->m_lrcLabel;
    m->m_lrcLabel = ui->label;
    QWidget::showFullScreen();
}

void lrcForm::closeEvent(QCloseEvent *)
{
    m->lrc->setLabelSize(m_labelSize);
    m->m_lrcLabel = m_label;
    m->m_lrcLabel->show();
}

void lrcForm::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    m->lrc->setLabelSize(ui->label->size());
}
