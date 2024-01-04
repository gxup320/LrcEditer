#include "batchprocessing.h"
#include "ui_batchprocessing.h"
#include "mainwindow.h"
#include <QStandardItemModel>
#include <QFileDialog>
#include <glrc.h>
#include <QMessageBox>

batchProcessing::batchProcessing(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::batchProcessing)
{
    ui->setupUi(this);
    ui->tableView_files->setShowGrid(true);
    ui->tableView_files->setGridStyle(Qt::DashLine);
    model_files = new QStandardItemModel;
    model_files->setHorizontalHeaderLabels({tr("files")});
    ui->tableView_files->setModel(model_files);
    ui->tableView_files->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView_files->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView_files->setEditTriggers(QAbstractItemView::NoEditTriggers);
    lrc = new GLrc;
}

batchProcessing::~batchProcessing()
{
    delete lrc;
    delete ui;
}

void batchProcessing::fileListAddItem(const QString &fileName)
{
    if(fileName != "")
    {
        //add file name to list
        QStandardItem* item = new QStandardItem(fileName);
        item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        int a = model_files->rowCount();
        model_files->setItem(a,0,item);
        ui->tableView_files->resizeColumnsToContents();
        ui->progressBar->setMaximum(model_files->rowCount());
    }
}

void batchProcessing::loadLrc(QString fileName)
{
    if(fileName != "")
    {
        QFile file(fileName);
        if(file.open(QFile::ReadOnly))
        {
            QString lrcFull = file.readAll();
            file.close();
            lrcFull.replace("\r\n", "\n");
            lrcFull.replace("\r", "\n");
            QStringList lrcLines = lrcFull.split("\n");
            lrcFull = "";
            al = "";
            ar = "";
            au = "";
            by = "";
            ti = "";
            re = "";
            ve = "";
            offset = 0;
            for (auto& lrcLine : lrcLines)
            {
                if(lrcLine.right(1) == "]")
                {
                    if(lrcLine.left(4) == "[al:")
                    {
                        al = lrcLine.mid(4,lrcLine.length() - 5);
                        continue;
                    }
                    else if(lrcLine.left(4) == "[ar:")
                    {
                        ar = lrcLine.mid(4,lrcLine.length() - 5);
                        continue;
                    }
                    else if(lrcLine.left(4) == "[au:")
                    {
                        au = lrcLine.mid(4,lrcLine.length() - 5);
                        continue;
                    }
                    else if(lrcLine.left(4) == "[by:")
                    {
                        by = lrcLine.mid(4,lrcLine.length() - 5);
                        continue;
                    }
                    else if(lrcLine.left(4) == "[ti:")
                    {
                        ti = lrcLine.mid(4,lrcLine.length() - 5);
                        continue;
                    }
                    else if(lrcLine.left(4) == "[re:")
                    {
                        re = lrcLine.mid(4,lrcLine.length() - 5);
                        continue;
                    }
                    else if(lrcLine.left(4) == "[ve:")
                    {
                        ve = lrcLine.mid(4,lrcLine.length() - 5);
                        continue;
                    }
                    else if(lrcLine.left(8) == "[offset:")
                    {
                        offset = lrcLine.mid(8,lrcLine.length() - 9).toInt();
                        continue;
                    }
                }
                if(lrcLine != "")
                {
                    lrcFull += lrcLine + "\n";
                }
            }
            lrc->setLrc(lrcFull);
            //if(re == "LrcEditer powered by GXUP320")
            //    on_pushButton_repair_clicked();
        }
    }
}

bool batchProcessing::saveLrcToFile(QString fileName)
{
    //ui->statusbar->showMessage(tr("Save lyric to ") + "'" + fileName +"'");
    QFile file(fileName);
    if(file.open(QFile::WriteOnly))
    {
        if(file.resize(0) == false)
            return false;
        QString lrcFull = "";
        if(ui->checkBox_al->isChecked())
        {
            al = ui->lineEdit_al->text();
        }
        if(al != "")
            lrcFull += "[al:" + al + "]\n";
        if(ui->checkBox_ar->isChecked())
        {
            ar = ui->lineEdit_ar->text();
        }
        if(ar != "")
            lrcFull += "[ar:" + ar + "]\n";
        if(ui->checkBox_au->isChecked())
        {
            au = ui->lineEdit_au->text();
        }
        if(au != "")
            lrcFull += "[au:" + au + "]\n";
        if(ui->checkBox_by->isChecked())
        {
            by = ui->lineEdit_by->text();
        }
        if(by != "")
            lrcFull += "[by:" + by + "]\n";
        if(ui->checkBox_re->isChecked())
        {
            re = ui->lineEdit_re->text();
        }
        if(re != "")
            lrcFull += "[re:" + re + "]\n";
        if(ui->checkBox_ti->isChecked())
        {
            ti = ui->lineEdit_ti->text();
        }
        if(ti != "")
            lrcFull += "[ti:" + ti + "]\n";
        if(ui->checkBox_ve->isChecked())
        {
            ve = ui->lineEdit_ve->text();
        }
        if(ve != "")
            lrcFull += "[ve:" + ve + "]\n";
        if(ui->checkBox_offset->isChecked())
        {
            offset = ui->spinBox_offset->value();
        }
        if(offset != 0)
            lrcFull += "[offset:" + QString::number(offset) + "]\n";
        if(ui->checkBox_deleteAllWordTime->isChecked())
        {
            lrc->deleteAllWordTime();
        }
        lrcFull += lrc->getLrc(ui->checkBox_moreTimes->isChecked());
        //lrcFull.replace("<br/>","\n");
        if(file.write(lrcFull.toUtf8()) == false)
        {
            //ui->statusbar->showMessage(tr("Save lyric to ") + "'" + fileName +"' fail!", 5000);
            return false;
        }
        file.close();
        //ui->statusbar->showMessage(tr("Save lyric to ") + "'" + fileName +"' success.", 5000);
        return true;
    }
    //ui->statusbar->showMessage(tr("Save lyric to ") + "'" + fileName +"' fail!", 5000);
    return false;
}

void batchProcessing::on_pushButton_add_file_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open LRC"), QString(), tr("LRC Files (*.lrc)"));
    fileListAddItem(fileName);
}


void batchProcessing::on_pushButton_add_directory_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this,
        tr("Open Directory"), nullptr, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(dir != "")
    {
        QDir list(dir);
        list.setFilter(QDir::Files|QDir::Readable);
        QStringList filters;
        filters << "*.lrc";
        list.setNameFilters(filters);
        QStringList fileList = list.entryList();
        for (auto& itm : fileList)
        {
            fileListAddItem(list.path() + "/" + itm);
        }
    }
}


void batchProcessing::on_pushButton_remove_clicked()
{
    QItemSelectionModel *select = ui->tableView_files->selectionModel();
    QModelIndexList models = select->selectedRows();
    if (models.size() == 1) {
            model_files->removeRow(models[0].row());
            ui->progressBar->setMaximum(model_files->rowCount());
    }
}


void batchProcessing::on_pushButton_outputSelect_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this,
        tr("Open Directory"), nullptr, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if(dir != "")
    {
        ui->lineEdit_output->setText(dir);
    }
}


void batchProcessing::on_pushButton_execute_clicked()
{
    QString saveDir = ui->lineEdit_output->text();
    if(saveDir == "")
    {
        QMessageBox::warning(this, tr("warning") ,tr("Please select an output directory"));
        return;
    }
    for (int var = 0; var < model_files->rowCount(); ++var)
    {
        QStandardItem* item=model_files->item(var);
        QString name = item->text();
        loadLrc(name);
        int i = name.lastIndexOf("/");
        qDebug() << name.mid(i);
        saveLrcToFile(saveDir + name.mid(i));
        ui->progressBar->setValue(var + 1);
    }
}

