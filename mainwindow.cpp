#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTime>
#include <QMediaMetaData>
#include <QFile>
#include <QMessageBox>
#include <QCloseEvent>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include "glrc.h"
#include "lineedit.h"
#include "lrcedit.h"
#include "lrcsearchneteasyform.h"
#include "batchprocessing.h"
#include "buffersizeedit.h"
#include "lrcform.h"
#include <QInputDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->label_cover->setAlignment(Qt::AlignCenter);
    qRegisterMetaType<QPixmap*>("QPixmap*");
    lrc = new GLrc;
    connect(lrc, SIGNAL(lrcChanged(int)), this, SLOT(lrcChanged(int)));
    ui->textEdit->setReadOnly(true);
    connect(ui->textEdit, SIGNAL(keyProc(QKeyEvent*)), this, SLOT(keyProc(QKeyEvent*)));
    connect(ui->textEdit, SIGNAL(focusOutProc(QFocusEvent*)), this, SLOT(focusOutProc(QFocusEvent*)));
    connect(ui->progressBar, SIGNAL(valFromMouse(qint64)), this, SLOT(valFromMouse(qint64)));
    connect(ui->openGLWidget_pcm, SIGNAL(valFromMouse(qint64)), this, SLOT(valFromMouse(qint64)));
    ui->textEdit->setFocus();
    setWindowTitle(tr("New File") + " - LrcEditer powered by GXUP320");

    player = new GAudioPlayer;
    connect(player, SIGNAL(durationChanged(qint64)), this, SLOT(durationChanged(qint64)));
    connect(player, SIGNAL(metaDataChanged(QMediaMetaData)), this, SLOT(metaDataChanged(QMediaMetaData)));
    connect(player, SIGNAL(positionChanged(qint64)), this, SLOT(positionChanged(qint64)), Qt::BlockingQueuedConnection);
    connect(player, SIGNAL(loadStatus(qint64,bool)), this, SLOT(loadStatus(qint64,bool)));
    connect(player, SIGNAL(bufferSizeChanged(qint64)), this, SLOT(bufferSizeChanged(qint64)));
    connect(player, SIGNAL(audioStateChanged(QAudio::State)), this, SLOT(audioStateChanged(QAudio::State)));

}

MainWindow::~MainWindow()
{
    disconnect(lrc, SIGNAL(lrcChanged(int)), this, SLOT(lrcChanged(int)));
    disconnect(player, SIGNAL(durationChanged(qint64)), this, SLOT(durationChanged(qint64)));
    disconnect(player, SIGNAL(metaDataChanged(QMediaMetaData)), this, SLOT(metaDataChanged(QMediaMetaData)));
    disconnect(player, SIGNAL(positionChanged(qint64)), this, SLOT(positionChanged(qint64)));
    disconnect(player, SIGNAL(loadStatus(qint64,bool)), this, SLOT(loadStatus(qint64,bool)));
    disconnect(player, SIGNAL(bufferSizeChanged(qint64)), this, SLOT(bufferSizeChanged(qint64)));
    QCoreApplication::processEvents();
    if(m_LrcSearchNeteasyForm != nullptr)
    {
        delete m_LrcSearchNeteasyForm;
    }
    if(m_batchProcessing != nullptr)
    {
        delete m_batchProcessing;
    }
    if(m_BufferSizeEdit != nullptr)
    {
        delete m_BufferSizeEdit;
    }
    if(m_lrcForm != nullptr)
    {
        delete m_lrcForm;
    }
    //delete audioOutput;
    //delete videoOutput;
    delete player;
    //delete mediaPlayer;
    delete ui;
    delete lrc;
}

void MainWindow::show()
{
    QWidget::show();
    ui->checkBox_pcm_pause->setCheckState(Qt::Checked);
    //ui->checkBox_lrc_pause->setCheckState(Qt::Checked);
}

void MainWindow::durationChanged(qint64 duration)
{
    ui->progressBar->setMaximum(duration);
    QTime tl = QTime::fromMSecsSinceStartOfDay(duration);
    QString length = tl.toString("mm:ss.") + tl.toString("zzz").left(2);
    ui->label_length->setText(length);
}

void MainWindow::positionChanged(qint64 position)
{
    //static int LastLine = -1;
    //static QString LastFillLrc;
    if(position > ui->progressBar->maximum())
        position = ui->progressBar->maximum();
    ui->progressBar->setValue(position);
    ui->openGLWidget_pcm->setTime(position);
    ui->openGLWidget_lrc->setTime(position);
    QTime tl = QTime::fromMSecsSinceStartOfDay(position);
    QString length = tl.toString("mm:ss.") + tl.toString("zzz").left(2);
    ui->pushButton_inst->setText(tr("inst") + " [" + length + "]");
    if(ui->checkBox_debug->isChecked())
    {
        displayLrc(position);
    }
    else
    {
        //LastLine = -1;
    }
}

void MainWindow::metaDataChanged(QMediaMetaData mediaData)
{
    QString str;
    QStringList strList;
    //QMediaMetaData mediaData = mediaPlayer->metaData();
    //qDebug() << "Author:" << mediaData.value(QMediaMetaData::Author).value<QStringList>();
    //qDebug() << "Comment:" << mediaData.stringValue(QMediaMetaData::Comment);
    //qDebug() << "Description:" << mediaData.stringValue(QMediaMetaData::Description);
    //qDebug() << "Genre:" << mediaData.value(QMediaMetaData::Genre).value<QStringList>();
    //qDebug() << "Date:" << mediaData.value(QMediaMetaData::Date).value<QDateTime>();
    //qDebug() << "Language:" << mediaData.Language(QMediaMetaData::Language).value<QLocale::Language>();
    //qDebug() << "Publisher:" << mediaData.stringValue(QMediaMetaData::Publisher);
    //qDebug() << "Copyright:" << mediaData.stringValue(QMediaMetaData::Copyright);
    //qDebug() << "Url:" << mediaData.value(QMediaMetaData::Url).value<QUrl>();
    //qDebug() << "MediaType:" << mediaData.stringValue(QMediaMetaData::MediaType);
    //qDebug() << "FileFormat:" << mediaData.stringValue(QMediaMetaData::FileFormat);
    //qDebug() << "Duration:" << mediaData.stringValue(QMediaMetaData::Duration);
    //qDebug() << "AudioBitRate:" << mediaData.stringValue(QMediaMetaData::AudioBitRate);
    //qDebug() << "AudioCodec:" << mediaData.stringValue(QMediaMetaData::AudioCodec);
    //qDebug() << "VideoFrameRate:" << mediaData.stringValue(QMediaMetaData::VideoFrameRate);
    //qDebug() << "VideoBitRate:" << mediaData.stringValue(QMediaMetaData::VideoBitRate);
    //qDebug() << "VideoCodec:" << mediaData.stringValue(QMediaMetaData::VideoCodec);
    //qDebug() << "AlbumTitle:" << mediaData.stringValue(QMediaMetaData::AlbumTitle);
    //qDebug() << "AlbumArtist:" << mediaData.stringValue(QMediaMetaData::AlbumArtist);
    //qDebug() << "ContributingArtist:" << mediaData.value(QMediaMetaData::ContributingArtist).value<QStringList>();
    //qDebug() << "TrackNumber:" << mediaData.stringValue(QMediaMetaData::TrackNumber);
    //qDebug() << "Composer:" << mediaData.value(QMediaMetaData::Composer).value<QStringList>();
    //qDebug() << "LeadPerformer:" << mediaData.value(QMediaMetaData::LeadPerformer).value<QStringList>();
    //qDebug() << "ThumbnailImage:" << mediaData.stringValue(QMediaMetaData::ThumbnailImage);
    //qDebug() << "CoverArtImage:" << mediaData.stringValue(QMediaMetaData::CoverArtImage);
    //qDebug() << "Orientation:" << mediaData.stringValue(QMediaMetaData::Orientation);
    //qDebug() << "Resolution:" << mediaData.stringValue(QMediaMetaData::Resolution);
    str = mediaData.stringValue(QMediaMetaData::Title);
    if(str != "")
        ui->lineEdit_ti->setText(str);
    str = mediaData.stringValue(QMediaMetaData::AlbumTitle);
    if(str != "")
        ui->lineEdit_al->setText(str);
    strList = mediaData.value(QMediaMetaData::Author).value<QStringList>();
    strList.removeDuplicates();
    if(strList.length() > 0)
    {
        str = strList[0];
        for(int i = 1; i < strList.length(); i++)
            str += " / " + strList[i];
    }
    else
    {
        str = "";
    }
    if(str != "")
        ui->lineEdit_ar->setText(str);
    strList = mediaData.value(QMediaMetaData::Composer).value<QStringList>();
    strList.removeDuplicates();
    if(strList.length() > 0)
    {
        str = strList[0];
        for(int i = 1; i < strList.length(); i++)
            str += " / " + strList[i];
    }
    else
    {
        str = "";
    }
    if(str != "")
        ui->lineEdit_au->setText(str);
    str = mediaData.stringValue(QMediaMetaData::MediaType);
    if(str == "video")
    {
        //videoOutput->show();
        ui->openGLWidget_lrc->setBackground(QColor(255,255,255));
        ui->openGLWidget_lrc->setDispaleColor(QColor(0,0,0), QColor(255,0,0), QColor(0,255,0), QColor(0,0,255));
    }
    else
    {
        //videoOutput->hide();
        QVariant img = mediaData.value(QMediaMetaData::ThumbnailImage);
        QPixmap cove =QPixmap::fromImage(img.value<QImage>());
        //ui->label_cover->setPixmap(cove.scaled(200,16777215,Qt::KeepAspectRatio, Qt::SmoothTransformation));
        if(img.isNull())
        {
            ui->openGLWidget_lrc->setBackground(QColor(255,255,255));
            ui->openGLWidget_lrc->setDispaleColor(QColor(0,0,0), QColor(255,0,0), QColor(0,255,0), QColor(0,0,255));
        }
        else
        {
            ui->openGLWidget_lrc->setBackground(img.value<QImage>());
            ui->openGLWidget_lrc->setDispaleColor(QColor(200,200,200,200), QColor(255,255,255,230), QColor(0,255,0,230), QColor(200,255,200,230));
        }
    }
}

void MainWindow::valFromMouse(qint64 val)
{
    player->setPosition(val);
}

void MainWindow::loadStatus(qint64 position, bool isEnd)
{
    //QTime time = QTime::fromMSecsSinceStartOfDay(position);
    QTime tl = QTime::fromMSecsSinceStartOfDay(position);
    QString length = tl.toString("mm:ss.") + tl.toString("zzz").left(2);
    if(isEnd)
    {
        //ui->label->setText("总长度："+time.toString("hh:mm:ss.zzz"));
        //ui->progressBar->setMaximum(position);
        ui->progressBar->setTextVisible(false);
        ui->progressBar->setMaximum(position);
        ui->label_length->setText(length);
        ui->openGLWidget_pcm->setPcm(player->getPcm());
        ui->openGLWidget_pcm->setLrc(lrc);
        ui->openGLWidget_lrc->setLrc(lrc);
        player->play();
        ui->statusbar->showMessage(tr("Music playing."), 3000);
    }
    else
    {
        //if(ui->progressBar->maximum() != 0)
        //{
        //    ui->progressBar->setValue(position);
        //    ui->progressBar->setTextVisible(true);
        //    ui->statusbar->showMessage(tr("Music loading") + " " + QString::number(position * 10000 / ui->progressBar->maximum() / 100.0) + "%");
        //}
        //ui->pushButton_inst->setText(tr("loading：")+("[" + length + "]"));
        if(position == -1)
        {
            ui->statusbar->showMessage(tr("Music decodeing..."));
        }
        else if(position == -2)
        {
            ui->statusbar->showMessage(tr("Audio device loading..."));
        }
        else if(position == -3)
        {
            ui->statusbar->showMessage(tr("Music loading..."));
        }
    }
}

void MainWindow::lrcChanged(int md)
{
    if(lrcFileName == "")
    {
        setWindowTitle("*" + tr("New File") + " - LrcEditer powered by GXUP320");
    }
    else
    {
        setWindowTitle("*" + lrcFileName + " - LrcEditer powered by GXUP320");
    }
    if(md != 0)
    {
        displayLrc(-1,true);
    }
}

void MainWindow::bufferSizeChanged(qint64 size)
{
    ui->pushButton_bufferSize->setText(QString::number(size));
}

void MainWindow::audioStateChanged(QAudio::State state)
{
    if(state == QAudio::IdleState)
    {
        if(ui->checkBox_loop->isChecked())
        {
            player->play();
        }
    }
}

bool MainWindow::saveLrcToFile(QString fileName)
{
    ui->statusbar->showMessage(tr("Save lyric to ") + "'" + fileName +"'");
    QFile file(fileName);
    if(file.open(QFile::WriteOnly))
    {
        if(file.resize(0) == false)
            return false;
        QString lrcFull = "";
        if(ui->lineEdit_al->text() != "")
            lrcFull += "[al:" + ui->lineEdit_al->text() + "]\n";
        if(ui->lineEdit_ar->text() != "")
            lrcFull += "[ar:" + ui->lineEdit_ar->text() + "]\n";
        if(ui->lineEdit_au->text() != "")
            lrcFull += "[au:" + ui->lineEdit_au->text() + "]\n";
        if(ui->lineEdit_by->text() != "")
            lrcFull += "[by:" + ui->lineEdit_by->text() + "]\n";
        lrcFull += "[re:LrcEditer powered by GXUP320]\n";
        if(ui->lineEdit_ti->text() != "")
            lrcFull += "[ti:" + ui->lineEdit_ti->text() + "]\n";
        lrcFull += "[ve:1.0]\n";
        lrcFull += lrc->getLrc(ui->checkBox_moreTimes->checkState());
        //lrcFull.replace("<br/>","\n");
        if(file.write(lrcFull.toUtf8()) == false)
        {
            ui->statusbar->showMessage(tr("Save lyric to ") + "'" + fileName +"' fail!", 5000);
            return false;
        }
        file.close();
        ui->statusbar->showMessage(tr("Save lyric to ") + "'" + fileName +"' success.", 5000);
        return true;
    }
    ui->statusbar->showMessage(tr("Save lyric to ") + "'" + fileName +"' fail!", 5000);
    return false;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(windowTitle().left(1) == "*")
    {
        QMessageBox msgBox;
        msgBox.setText(tr("The document has been modified."));
        msgBox.setInformativeText(tr("Do you want to save your changes?"));
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        int ret = msgBox.exec();
        switch (ret) {
          case QMessageBox::Save:
              // Save was clicked
              on_pushButton_save_clicked();
              if(windowTitle().left(1) == "*")
                  event->ignore();
              break;
          case QMessageBox::Discard:
              // Don't Save was clicked
              break;
          case QMessageBox::Cancel:
              // Cancel was clicked
              event->ignore();
              break;
          default:
              // should never be reached
              event->ignore();
              break;
        }
    }
}

void MainWindow::displayLrc(qint64 time, bool f)
{
    static qint64 lestLocalTime = 0;
    qint64 localTime;
    int line;
    QString html = lrc->getHtml(time ,true , &localTime, &line, f);
    if(f || lestLocalTime != localTime)
    {
        lestLocalTime = localTime;
        ui->textEdit->setText(html);
        QTextCursor c = ui->textEdit->textCursor();
        QString lrcString = ui->textEdit->toPlainText();
        QStringList lrcLines = lrcString.split("\n");
        int pos = 0;
        for(int i = 0; i < line + 5 && i < lrcLines.size(); i++)
        {
            pos += lrcLines[i].size() + 1;
        }
        if(pos > lrcString.size())
            pos = lrcString.size();
        c.setPosition(pos);
        ui->textEdit->setTextCursor(c);
    }
}

void MainWindow::loadLrc(QString fileName)
{
    if(fileName != "")
    {
        QFile file(fileName);
        if(file.open(QFile::ReadOnly))
        {
            lrcFileName = fileName;
            QString lrcFull = file.readAll();
            file.close();
            lrcFull.replace("\r\n", "\n");
            lrcFull.replace("\r", "\n");
            QStringList lrcLines = lrcFull.split("\n");
            lrcFull = "";
            QString re;
            //for (auto& lrcLine : lrcLines)
            for(int var = 0; var < lrcLines.length(); var++)
            {
                auto& lrcLine = lrcLines[var];
                if(lrcLine.right(1) == "]")
                {
                    if(lrcLine.left(4) == "[al:")
                    {
                        ui->lineEdit_al->setText(lrcLine.mid(4,lrcLine.length() - 5));
                        continue;
                    }
                    else if(lrcLine.left(4) == "[ar:")
                    {
                        ui->lineEdit_ar->setText(lrcLine.mid(4,lrcLine.length() - 5));
                        continue;
                    }
                    else if(lrcLine.left(4) == "[au:")
                    {
                        ui->lineEdit_au->setText(lrcLine.mid(4,lrcLine.length() - 5));
                        continue;
                    }
                    else if(lrcLine.left(4) == "[by:")
                    {
                        ui->lineEdit_by->setText(lrcLine.mid(4,lrcLine.length() - 5));
                        continue;
                    }
                    else if(lrcLine.left(4) == "[ti:")
                    {
                        ui->lineEdit_ti->setText(lrcLine.mid(4,lrcLine.length() - 5));
                        continue;
                    }
                    else if(lrcLine.left(4) == "[re:")
                    {
                        re = lrcLine.mid(4,lrcLine.length() - 5);
                        continue;
                    }
                    else if(lrcLine.left(4) == "[ve:")
                    {

                        continue;
                    }
                    else if(lrcLine.left(8) == "[offset:")
                    {
                        QMessageBox::warning(this, tr("warning") ,tr("offset option ignored"));
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
            setWindowTitle(lrcFileName + " - LrcEditer powered by GXUP320");
            displayLrc(-1, true);
        }
    }
}

qint64 MainWindow::getBufferSize()
{
    return player->getBufferSize();
}

qint64 MainWindow::getBufferSizeSmall()
{
    return player->getBufferSizeSmall();
}

qint64 MainWindow::setBufferSize(qint64 size, qint64 sizeSmall)
{
    return player->setBufferSize(size, sizeSmall);
}

void MainWindow::on_pushButton_loadmusic_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this);
    if(fileName != "")
    {
        //检查同名歌词
        qsizetype index1 = fileName.lastIndexOf(".");
        qsizetype index2 = fileName.lastIndexOf("\\");
        qsizetype index3 = fileName.lastIndexOf("/");
        if(index1 >= index2 || index1 >= index3)
        {
            loadLrc(fileName.left(index1) + ".lrc");
        }
        else
        {
            loadLrc(fileName + ".lrc");
        }
        ui->statusbar->showMessage(tr("Load music from ") + "'" + fileName +"'");
        player->stop();
        ui->progressBar->setValue(0);
        ui->progressBar->setMaximum(0);
        //mediaPlayer->setSource(QUrl::fromLocalFile(fileName));
        player->load(QUrl::fromLocalFile(fileName));
        //player->play();
    }
}


void MainWindow::on_pushButton_tostart_clicked()
{
    if(player != nullptr)
    {
        player->setPosition(0);
    }
}



void MainWindow::on_pushButton_play_pause_clicked()
{
    if(player != nullptr)
    {
        if(player->isPlaying())
        {
            player->stop();
        }
        else
        {
            player->play();
        }
    }
}


void MainWindow::on_pushButton_loadlrc_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open LRC"), QString(), tr("LRC Files (*.lrc)"));
    loadLrc(fileName);
}


void MainWindow::on_pushButton_save_clicked()
{
    if(lrcFileName == "")
    {
        on_pushButton_save_as_clicked();
    }
    else
    {
        if(saveLrcToFile(lrcFileName))
        {
            setWindowTitle(lrcFileName + " - LrcEditer powered by GXUP320");
        }
        else
        {
            QMessageBox::critical(this,tr("Save fail"),tr("Save to ") + "'" + lrcFileName + "'" + tr(" fail."));
        }
    }
}


void MainWindow::on_pushButton_save_as_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Open LRC"), lrcFileName, tr("LRC Files (*.lrc);;SubRip (*.srt)"));
    if(fileName != "")
    {
        if(fileName.right(4).toLower() == ".lrc")
        {
            if(saveLrcToFile(fileName))
            {
                lrcFileName = fileName;
                setWindowTitle(lrcFileName + " - LrcEditer powered by GXUP320");
            }
            else
            {
                QMessageBox::critical(this,tr("Save fail"),tr("Save to ") + "'" + fileName + "'" + tr(" fail."));
            }
        }
        else if(fileName.right(4).toLower() == ".srt")
        {
            QFile file(fileName);
            if(file.open(QFile::WriteOnly))
            {
                file.write(lrc->toSrt().toUtf8());
                setWindowTitle(lrcFileName + " - LrcEditer powered by GXUP320");
            }
            else
            {
                QMessageBox::critical(this,tr("Save fail"),tr("Save to ") + "'" + fileName + "'" + tr(" fail."));
            }
        }
    }
}


void MainWindow::on_pushButton_inst_clicked()
{
    lrc->instTime(player->position());
    lrc->nextLine();
    displayLrc(-1, true);
}


void MainWindow::on_pushButton_remove_clicked()
{
    lrc->removeTime();
    displayLrc(-1, true);
}


bool MainWindow::keyProc(QKeyEvent *event)
{
    if(ui->checkBox_keyop->isChecked())
    {
        if(event->key() == Qt::Key_Up)
        {
            qint64 time = lrc->previousItem();
            if(time != -1 && player != nullptr)
                player->setPosition(time);
            displayLrc(-1, true);
        }
        else if(event->key() == Qt::Key_Down)
        {
            qint64 time = lrc->nextItem();
            if(time != -1 && player != nullptr)
                player->setPosition(time);
            displayLrc(-1, true);
        }
        else if(event->key() == Qt::Key_Left)
        {
            if(event->modifiers() == (Qt::ControlModifier))
            {
                qint64 time = lrc->timeAdd(-10);
                player->setPosition(time);
                displayLrc(-1, true);
            }
            else if(event->modifiers() == (Qt::AltModifier))
            {
                qint64 time = lrc->timeAdd(-100);
                player->setPosition(time);
                displayLrc(-1, true);
            }
            else
            {
                if(player != nullptr)
                    player->setPosition(player->position() - 3000);
            }
        }
        else if(event->key() == Qt::Key_Right)
        {
            if(event->modifiers() == (Qt::ControlModifier))
            {
                qint64 time = lrc->timeAdd(10);
                player->setPosition(time);
                displayLrc(-1, true);
            }
            else if(event->modifiers() == (Qt::AltModifier))
            {
                qint64 time = lrc->timeAdd(100);
                player->setPosition(time);
                displayLrc(-1, true);
            }
            else
            {
                if(player != nullptr)
                {
                    qint64 time = player->position() + 3000;
                    if(time > ui->progressBar->maximum() - 100)
                        time = ui->progressBar->maximum() - 100;
                    player->setPosition(time);
                }
            }
        }
        else if(event->key() == Qt::Key_P)
        {
            on_pushButton_play_pause_clicked();
        }
        else if(event->key() == Qt::Key_Space)
        {
            on_pushButton_inst_clicked();
        }
        else if(event->key() == Qt::Key_Backspace)
        {
            on_pushButton_remove_clicked();
        }
        else if(event->key() == Qt::Key_E)
        {
            on_pushButton_edit_line_clicked();
        }
        else if(event->key() == Qt::Key_D)
        {
            ui->checkBox_debug->setChecked(!ui->checkBox_debug->isChecked());
        }
        else if(event->key() == Qt::Key_A)
        {
            ui->checkBox_accurate_process->setChecked(!ui->checkBox_accurate_process->isChecked());
        }
        else if(event->key() == Qt::Key_B)
        {
            if(event->modifiers() == (Qt::ControlModifier))
            {
                qint64 time = lrc->wordTimeAdd(-10);
                player->setPosition(time);
            }
            else if(event->modifiers() == (Qt::AltModifier))
            {
                qint64 time = lrc->wordTimeAdd(-100);
                player->setPosition(time);
            }
            else
            {
                qint64 time = lrc->previousWord();
                if(time != -1)
                    player->setPosition(time);
            }
            displayLrc(-1, true);
        }
        else if(event->key() == Qt::Key_N)
        {
            if(event->modifiers() == (Qt::ControlModifier))
            {
                qint64 time = lrc->wordTimeAdd(10);
                player->setPosition(time);
            }
            else if(event->modifiers() == (Qt::AltModifier))
            {
                qint64 time = lrc->wordTimeAdd(100);
                player->setPosition(time);
            }
            else
            {
                qint64 time = lrc->nextWord();
                if(time != -1)
                    player->setPosition(time);
            }
            displayLrc(-1, true);
        }
        else if(event->key() == Qt::Key_M)
        {
            lrc->setWordTime(player->position());
            int s = lrc->getSelectWord();
            lrc->nextWord();
            if(lrc->getSelectWord() == s)
            {
                lrc->nextLine();
                lrc->selectWordId(0);
            }
            displayLrc(-1, true);
        }
        else if(event->key() == Qt::Key_C)
        {
            on_pushButton_deleteLineWordTime_clicked();
        }
        return false;
    }
    else
    {
        if(event->key() == Qt::Key_Escape)
        {
            ui->checkBox_keyop->setChecked(true);
            displayLrc(-1, true);
            return false;
        }
        return true;
    }

}

void MainWindow::focusOutProc(QFocusEvent *)
{
    if(ui->lineEdit_backms->hasFocus() == false &&
            ui->lineEdit_al->hasFocus() == false &&
            ui->lineEdit_ar->hasFocus() == false &&
            ui->lineEdit_au->hasFocus() == false &&
            ui->lineEdit_by->hasFocus() == false &&
            ui->lineEdit_ti->hasFocus() == false)
        ui->textEdit->setFocus();
}

void MainWindow::on_pushButton_sort_clicked()
{
    lrc->lrcTimesSort();
    displayLrc(-1, true);
}


void MainWindow::on_pushButton_loadlrc_neteasy_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open NetEasy Music LRC Temp File"), QString("C:/Users/Administrator/AppData/Local/Netease/CloudMusic/Temp"));
    if(fileName != "")
    {
        QFile file(fileName);
        if(file.open(QFile::ReadOnly))
        {
            QStringList lrclist;
            QJsonObject obj = QJsonDocument::fromJson(file.readAll()).object();
            if(ui->checkBox_nmlrc->isChecked())
            {
                QJsonObject lrcobj = obj.find("lrc")->toObject();
                lrclist << lrcobj.find("lyric")->toString().split("\n");
                lrclist << obj.find("lyric")->toString().split("\n");
            }
            if(ui->checkBox_nmtlrc->isChecked())
            {
                QJsonObject lrcobj = obj.find("tlyric")->toObject();
                lrclist << lrcobj.find("lyric")->toString().split("\n");
                lrclist << obj.find("translateLyric")->toString().split("\n");
            }
            if(ui->checkBox_nmrlrc->isChecked())
            {
                QJsonObject lrcobj = obj.find("romalrc")->toObject();
                lrclist << lrcobj.find("lyric")->toString().split("\n");
                lrclist << obj.find("romeLrc")->toString().split("\n");
            }
            if(lrclist.length() != 0)
            {
                QString fullLrc = lrclist[0];
                for(int i = 1; i < lrclist.length(); i++)
                    fullLrc += "\n" + lrclist[i];
                //ui->textEdit->setPlainText(fullLrc);
                lrc->setLrc(fullLrc);
                on_pushButton_sort_clicked();
            }
        }
    }
}

void MainWindow::on_horizontalSlider_valueChanged(int value)
{
    qreal linearVolume = QAudio::convertVolume(value / qreal(100.0),
                                               QAudio::LogarithmicVolumeScale,
                                               QAudio::LinearVolumeScale);
    player->setVolume(linearVolume);
    ui->label_vol->setText(QString::number(value));
}



void MainWindow::on_pushButton_merge_duplicates_clicked()
{
    lrc->mergeDuplicates();
    displayLrc(-1, true);
}


void MainWindow::on_pushButton_edit_line_clicked()
{
    if(lrc->getSelectLine() >= lrc->size() || lrc->getSelectLine() < 0)
        return;
    if(linee != nullptr)
        delete linee;
    linee = new LineEdit;
    linee->m = this;
    linee->exec();
}


void MainWindow::on_pushButton_edit_lrc_clicked()
{
    if(lrce != nullptr)
        delete lrce;
    lrce = new LrcEdit;
    lrce->m = this;
    lrce->exec();
}


void MainWindow::on_pushButton_inst_line_clicked()
{
    lrc->instLine();
    lrc->instTime(player->position());
    on_pushButton_edit_line_clicked();
    displayLrc(-1, true);
}


void MainWindow::on_pushButton_append_line_clicked()
{

    int select = lrc->instLine(lrc->size());
    lrc->setSelectLine(select);
    lrc->instTime(player->position());
    on_pushButton_edit_line_clicked();
    displayLrc(-1, true);
}


void MainWindow::on_pushButton_search_from_neteasy_clicked()
{
    ui->pushButton_search_from_neteasy->setEnabled(false);
    if(m_LrcSearchNeteasyForm != nullptr)
        delete m_LrcSearchNeteasyForm;
    m_LrcSearchNeteasyForm = new LrcSearchNeteasyForm;
    m_LrcSearchNeteasyForm->m = this;
    m_LrcSearchNeteasyForm->exec();
    ui->pushButton_search_from_neteasy->setEnabled(true);
}


void MainWindow::on_pushButton_remove_line_clicked()
{
    lrc->removeLine();
    displayLrc(-1, true);
}


void MainWindow::on_checkBox_accurate_process_stateChanged(int arg1)
{
    player->accurateProgress(arg1);
}


void MainWindow::on_pushButto_batchProcess_clicked()
{
    ;
    if(m_batchProcessing != nullptr)
        delete m_batchProcessing;
    m_batchProcessing = new batchProcessing;
    m_batchProcessing->exec();
}


void MainWindow::on_checkBox_debug_stateChanged(int arg1)
{
    if(arg1 == 0)
    {
        displayLrc(-1, true);
    }
}


void MainWindow::on_pushButton_deleteWordTime_clicked()
{
    lrc->deleteWordTime();
}


void MainWindow::on_pushButton_deleteLineWordTime_clicked()
{
    lrc->deleteLineWordTime();
    if(!ui->checkBox_debug->isChecked())
    {
        lrc->selectWordId(0);
        qint64 time = lrc->getSelectTime();
        if(time != -1 && player != nullptr)
            player->setPosition(time);
    }
    displayLrc(-1, true);
}


void MainWindow::on_pushButton_deleteAllWordTime_clicked()
{
    lrc->deleteAllWordTime();
}


void MainWindow::on_pushButton_bufferSize_clicked()
{
    if(m_BufferSizeEdit != nullptr)
        delete m_BufferSizeEdit;
    m_BufferSizeEdit = new BufferSizeEdit;
    m_BufferSizeEdit->m = this;
    m_BufferSizeEdit->exec();
}


void MainWindow::on_pushButton_showLrcWindow_clicked()
{
    if(m_lrcForm != nullptr)
        delete m_lrcForm;
    m_lrcForm = new lrcForm;
    ui->openGLWidget_lrc->copyTo(m_lrcForm->lrcWindow);
    m_lrcForm->show();
}


void MainWindow::on_pushButtonfullScreen_clicked()
{
    if(m_lrcForm != nullptr)
        delete m_lrcForm;
    m_lrcForm = new lrcForm;
    ui->openGLWidget_lrc->copyTo(m_lrcForm->lrcWindow);
    m_lrcForm->showFullScreen();
}


void MainWindow::on_pushButton_time_offset_clicked()
{
    int val = QInputDialog::getInt(this, tr("input offset time"), tr("time(ms):"));
    if(val != 0)
    {
        lrc->timeAddAll(val);
        displayLrc(-1, true);
    }
}

void MainWindow::on_checkBox_lrc_pause_stateChanged(int arg1)
{
    if(arg1 == Qt::Checked)
    {
        ui->openGLWidget_lrc->pause(true);
        //ui->openGLWidget_lrc->hide();
    }
    else
    {
        ui->openGLWidget_lrc->pause(false);
        //ui->openGLWidget_lrc->show();
    }
}


void MainWindow::on_checkBox_pcm_pause_stateChanged(int arg1)
{
    if(arg1 == Qt::Checked)
    {
        ui->openGLWidget_pcm->pause(true);
        ui->openGLWidget_pcm->hide();
    }
    else
    {
        ui->openGLWidget_pcm->pause(false);
        ui->openGLWidget_pcm->show();
    }
}

