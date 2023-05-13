#include "lineedit.h"
#include "ui_lineedit.h"
#include "mainwindow.h"
#include "glrc.h"
#include <QMessageBox>

LineEdit::LineEdit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LineEdit)
{
    ui->setupUi(this);
}

LineEdit::~LineEdit()
{
    delete ui;
}

void LineEdit::show()
{
    selectLine = m->lrc->getSelectLine();
    ui->lineEdit->setText(m->lrc->getTimes(selectLine));
    ui->plainTextEdit->setPlainText(m->lrc->getLine(selectLine));
    QWidget::show();
}

bool LineEdit::close()
{
    return QWidget::close();
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

void LineEdit::closeEvent(QCloseEvent *)
{
    m->show();
}

