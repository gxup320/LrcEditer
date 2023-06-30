#include "buffersizeedit.h"
#include "ui_buffersizeedit.h"
#include "mainwindow.h"

BufferSizeEdit::BufferSizeEdit(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BufferSizeEdit)
{
    ui->setupUi(this);
}

BufferSizeEdit::~BufferSizeEdit()
{
    delete ui;
}

void BufferSizeEdit::show()
{
    ui->lineEdit_bufferEdit->setText(QString::number(m->getBufferSize()));
    ui->lineEdit_bufferEditSmall->setText(QString::number(m->getBufferSizeSmall()));
    QWidget::show();
}

void BufferSizeEdit::on_BufferSizeEdit_rejected()
{
    m->show();
}


void BufferSizeEdit::on_BufferSizeEdit_accepted()
{
    QString str = ui->lineEdit_bufferEdit->text();
    QString strSmall = ui->lineEdit_bufferEditSmall->text();
    m->setBufferSize(str.toLongLong(), strSmall.toLongLong());
    m->show();
}


void BufferSizeEdit::on_buttonBox_accepted()
{

}

