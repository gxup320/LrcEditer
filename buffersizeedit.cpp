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

int BufferSizeEdit::exec()
{
    ui->lineEdit_bufferEdit->setText(QString::number(m->getBufferSize()));
    ui->lineEdit_bufferEditSmall->setText(QString::number(m->getBufferSizeSmall()));
    return QDialog::exec();
}

void BufferSizeEdit::on_BufferSizeEdit_accepted()
{
    QString str = ui->lineEdit_bufferEdit->text();
    QString strSmall = ui->lineEdit_bufferEditSmall->text();
    m->setBufferSize(str.toLongLong(), strSmall.toLongLong());
}

