#include "lineedit.h"
#include "ui_lineedit.h"
#include "mainwindow.h"
#include "glrc.h"
#include <QMessageBox>

LineEdit::LineEdit(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LineEdit)
{
    ui->setupUi(this);
}

LineEdit::~LineEdit()
{
    delete ui;
}

int LineEdit::exec()
{
    selectLine = m->lrc->getSelectLine();
    ui->lineEdit->setText(m->lrc->getTimes(selectLine));
    ui->plainTextEdit->setPlainText(m->lrc->getLineString(selectLine));
    return QDialog::exec();
}

void LineEdit::on_pushButton_save_clicked()
{
    m->lrc->setLine(selectLine, ui->plainTextEdit->toPlainText());
    m->lrc->setTimes(selectLine, ui->lineEdit->text());
    m->displayLrc(-1, true);
    close();
}

void LineEdit::on_pushButton_close_clicked()
{
    close();
}
