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
    QWidget::show();
}

void BufferSizeEdit::on_BufferSizeEdit_rejected()
{
    m->show();
}


void BufferSizeEdit::on_BufferSizeEdit_accepted()
{
    QString str = ui->lineEdit_bufferEdit->text();
    m->setBufferSize(str.toLongLong());
    m->show();
}

