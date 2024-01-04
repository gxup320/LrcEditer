#include "lrcedit.h"
#include "ui_lrcedit.h"
#include "mainwindow.h"
#include "glrc.h"
#include <QMessageBox>

LrcEdit::LrcEdit(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LrcEdit)
{
    ui->setupUi(this);
}

LrcEdit::~LrcEdit()
{
    delete ui;
}

int LrcEdit::exec()
{
    ui->plainTextEdit->setPlainText(m->lrc->getLrc());
    return QDialog::exec();
}

void LrcEdit::on_pushButton_save_clicked()
{
    m->lrc->setLrc(ui->plainTextEdit->toPlainText(), ui->lineEdit->text().toInt());
    m->displayLrc(-1, true);
    close();
}


void LrcEdit::on_pushButton_close_clicked()
{
    close();
}
