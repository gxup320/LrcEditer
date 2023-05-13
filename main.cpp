#include "mainwindow.h"

#include <QApplication>
#include <QTranslator>
#include <QMessageBox>
#include <QtGlobal>
#include <QProcessEnvironment>
#include <QFile>
#include <QLibraryInfo>

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
#ifdef Q_OS_WIN
    //QT6 bug for windows
    QString path = QProcessEnvironment::systemEnvironment().value("SystemRoot");
    path += "\\System32\\mfplat.dll";
    //qDebug() << QCoreApplication::applicationDirPath() + "\\mfplat.dll.dll";
    QFile::copy(path, a.applicationDirPath() + "/mfplat.dll.dll");
#endif
    QTranslator translator;
    QLocale locale;
    //qDebug() << locale.name();
    if(translator.load("language/" + locale.name() + ".qm"))
    {
        a.installTranslator(&translator);
    }
    else
    {
        QMessageBox::warning(nullptr,"language load fail.","language load fail.\ncon't open 'language/" + locale.name() + ".qm'");
    }
    MainWindow w;
    w.show();
    return a.exec();
}
