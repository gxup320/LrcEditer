#include "lrcsearchneteasyform.h"
#include "ui_lrcsearchneteasyform.h"
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include "mainwindow.h"
#include "glrc.h"
#include "ui_mainwindow.h"
#include <QMessageBox>

LrcSearchNeteasyForm::LrcSearchNeteasyForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LrcSearchNeteasyForm)
{
    ui->setupUi(this);
    musicArr = new QJsonArray;
}

LrcSearchNeteasyForm::~LrcSearchNeteasyForm()
{
    delete musicArr;
    delete ui;
}

void LrcSearchNeteasyForm::show()
{
    if(m->netEasyApiUrl != "")
        ui->lineEdit_url->setText(m->netEasyApiUrl);
    ui->lineEdit->setText(m->ui->lineEdit_ti->text());
    QWidget::show();
    QCoreApplication::processEvents();
    if(ui->lineEdit->text() != "")
        wangyiyunList(ui->lineEdit->text());
}

bool LrcSearchNeteasyForm::close()
{
    return QWidget::close();
}

bool LrcSearchNeteasyForm::wangyiyunList(QString name)
{
    //搜索
    QString url = ui->lineEdit_url->text() + "/cloudsearch?keywords=" + name;
    QNetworkRequest Request = QNetworkRequest(url);
    QNetworkReply *reply;
    QNetworkAccessManager HTTPManger;
    reply = HTTPManger.get(Request);
    QEventLoop eventLoop;
    QObject::connect(reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    eventLoop.exec();
    if (reply->error() == QNetworkReply::NoError)
    {
        //qDebug() << "request protobufHttp NoError";
    }
    else
    {
        qDebug()<<"request protobufHttp handle errors here";
        QVariant statusCodeV = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        qDebug( "request protobufHttp found error ....code: %d %d\n", statusCodeV.toInt(), (int)reply->error());
        qDebug() << qPrintable(reply->errorString());
        return false;
    }
    QJsonDocument target = QJsonDocument::fromJson(reply->readAll());
    reply->deleteLater();
    QJsonObject retObj = target.object();
    if(retObj.find("result") == retObj.end())
    {
        return false;
    }
    QJsonObject result = retObj.find("result")->toObject();
    if(result.find("songCount") == result.end())
    {
        return false;
    }
    int songCount = result.find("songCount")->toInt();
    if(songCount < 1 || result.find("songs") == result.end())
    {
        return false;
    }
    QJsonArray songs = result.find("songs")->toArray();
    if(songs.size() < 1)
    {
        return false;
    }
    while(musicArr->count())
    {
        musicArr->pop_back();
    }
    for(int i = 0; i < 50 && i < songs.size(); i++)
    {
        QJsonObject songsObj = songs[i].toObject();
        int id = 0;
        QString name;
        if(songsObj.find("id") != songsObj.end())
            id = songsObj.find("id")->toInt();
        if(songsObj.find("name") != songsObj.end())
            name = songsObj.find("name")->toString();
        QString artists = "";
        if(songsObj.find("artists") != songsObj.end())
        {
            QJsonArray artistsarr = songsObj.find("artists")->toArray();
            for(auto itm : artistsarr)
            {
                QJsonObject artistsObj = itm.toObject();
                if(artistsObj.find("name") != artistsObj.end())
                {
                    if(artists == "")
                    {
                        artists += artistsObj.find("name")->toString();
                    }
                    else
                    {
                        artists += "/" + artistsObj.find("name")->toString();
                    }
                }
            }
        }
        qDebug() << artists;
        QString alias = "";
        QJsonArray aliasarr;
        if(songsObj.find("alias") != songsObj.end())
               aliasarr = songsObj.find("alias")->toArray();
        if(aliasarr.size() > 0)
        {
            alias = aliasarr[0].toString();
        }
        if(alias != "")
        {
            name += "(" + alias + ")";
        }
        if(artists != "")
        {
            name += " - " + artists;
        }
        QJsonObject obj;
        obj.insert("id", id);
        obj.insert("name", name);
        musicArr->insert(musicArr->size(),obj);
        ui->comboBox->insertItem(ui->comboBox->count(),name);
    }
    return true;
}

void LrcSearchNeteasyForm::lrcSort(QString &fullLrc)
{
    QList<lrcTime> lrcTimes;
    lrcTime lrcTimeTemp;
    QStringList lrcLines = fullLrc.split("\n");
    for (int i = 0; i < lrcLines.length(); i++) {
        qint64 time = getTimeOfLrcLine(lrcLines[i]);
        while(time != -1)
        {
            lrcTimeTemp.line = i;
            lrcTimeTemp.time = time;
            lrcTimes.append(lrcTimeTemp);
            time = getTimeOfLrcLine(lrcLines[i]);
        }
    }
    lrcTimesSort(lrcTimes);
    int LastLine = -1;
    //QString fullLrc;
    fullLrc = "";
    QString tempLrc;
    for(auto& lrcTime:lrcTimes)
    {
        if(lrcTime.time == -1 || lrcTime.time != LastLine)
        {
            LastLine = lrcTime.time;
            if(fullLrc == "")
                fullLrc = tempLrc;
            else
                fullLrc += "\n" + tempLrc;
            if(lrcTime.time >= 0)
            {
                QTime tl = QTime::fromMSecsSinceStartOfDay(lrcTime.time);
                tempLrc = "[" + tl.toString("mm:ss.") + tl.toString("zzz").left(2) + "]" + lrcLines[lrcTime.line];
            }
            else
            {
                tempLrc = lrcLines[lrcTime.line];
            }
        }
        else if(lrcLines[lrcTime.line] != "")
        {
            tempLrc += "<br/>" + lrcLines[lrcTime.line];
        }
    }
    if(fullLrc == "")
        fullLrc = tempLrc;
    else
        fullLrc += "\n" + tempLrc;
    //ui->textEdit->setPlainText(fullLrc);
}

qint64 LrcSearchNeteasyForm::getTimeOfLrcLine(QString &lrcLine)
{
    if(lrcLine.left(1) != "[")
    {
        return -1;
    }
    if(lrcLine.indexOf("]") == -1)
    {
        return -1;
    }
    QString timeStr = lrcLine.mid(1,lrcLine.indexOf("]") - 1);
    //qDebug() << timeStr;
    QTime time = QTime::fromString(timeStr + "0","mm:ss.zzz");
    if(time.isNull())
    {
        time = QTime::fromString(timeStr,"mm:ss.zzz");
    }
    if(time.isNull())
    {
        return -1;
    }
    lrcLine = lrcLine.right(lrcLine.length() - lrcLine.indexOf("]") - 1);
    return time.msecsSinceStartOfDay();
}

void LrcSearchNeteasyForm::lrcTimesSort(QList<lrcTime> &lrcTimes)
{
    int i, j;
    for (i = 0; i < lrcTimes.length() - 1; i++)
        for (j = 0; j < lrcTimes.length() - 1 - i; j++)
            if (lrcTimes[j].time > lrcTimes[j + 1].time)
                lrcTimes.swapItemsAt(j, j + 1);
}

void LrcSearchNeteasyForm::closeEvent(QCloseEvent *)
{
    m->show();
}

void LrcSearchNeteasyForm::on_comboBox_currentIndexChanged(int index)
{
    if(index < musicArr->count())
    {
        QString url = ui->lineEdit_url->text() + "/lyric?id=" + QString::number((*musicArr)[index].toObject().find("id")->toInt());
        QNetworkRequest Request = QNetworkRequest(url);
        QNetworkReply *reply;
        QNetworkAccessManager HTTPManger;
        reply = HTTPManger.get(Request);
        QEventLoop eventLoop;
        QObject::connect(reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
        eventLoop.exec();
        if (reply->error() == QNetworkReply::NoError)
        {
            //qDebug() << "request protobufHttp NoError";
        }
        else
        {
            qDebug()<<"request protobufHttp handle errors here";
            QVariant statusCodeV = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
            qDebug( "request protobufHttp found error ....code: %d %d\n", statusCodeV.toInt(), (int)reply->error());
            qDebug() << qPrintable(reply->errorString());
            return;
        }
        QJsonObject obj = QJsonDocument::fromJson(reply->readAll()).object();
        reply->deleteLater();
        QStringList lrclist;
        if(obj.find("lrc") == obj.end())
        {
            return;
        }
        QJsonObject lrcobj = obj.find("lrc")->toObject();
        if(lrcobj.find("lyric") == lrcobj.end())
        {
            return;
        }
        lrclist << lrcobj.find("lyric")->toString().split("\n");
        if(obj.find("tlyric") == obj.end())
        {
            return;
        }
        QJsonObject tlrcobj = obj.find("tlyric")->toObject();
        if(tlrcobj.find("lyric") == tlrcobj.end())
        {
            return;
        }
        lrclist << tlrcobj.find("lyric")->toString().split("\n");
        QString fullLrc;
        if(lrclist.length() != 0)
        {
            fullLrc = lrclist[0];
            for(int i = 1; i < lrclist.length(); i++)
                fullLrc += "\n" + lrclist[i];
        }
        qDebug() << fullLrc;
        lrcSort(fullLrc);
        if(fullLrc == "")
            return;
        fullLrc.replace("<br/>","\n");
        ui->plainTextEdit->setPlainText(fullLrc);
        return;
    }
}

void LrcSearchNeteasyForm::on_pushButton_search_clicked()
{
    ui->comboBox->clear();
    wangyiyunList(ui->lineEdit->text());
}


void LrcSearchNeteasyForm::on_pushButton_apply_clicked()
{
    m->lrc->setLrc(ui->plainTextEdit->toPlainText(), ui->lineEdit->text().toInt());
    m->displayLrc(-1, true);
    close();
}


void LrcSearchNeteasyForm::on_pushButton_close_clicked()
{
    close();
}



void LrcSearchNeteasyForm::on_lineEdit_url_textChanged(const QString &arg1)
{
    m->netEasyApiUrl = arg1;
}

