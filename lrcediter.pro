QT       += core gui
QT       += multimedia
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

TRANSLATIONS += "language/Chinese.ts"

SOURCES += \
    batchprocessing.cpp \
    gaudioplayer.cpp \
    glrc.cpp \
    gprogressbar.cpp \
    lineedit.cpp \
    lrcedit.cpp \
    lrcqtextedit.cpp \
    lrcsearchneteasyform.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    batchprocessing.h \
    gaudioplayer.h \
    glrc.h \
    gprogressbar.h \
    lineedit.h \
    lrcedit.h \
    lrcqtextedit.h \
    lrcsearchneteasyform.h \
    mainwindow.h

FORMS += \
    batchprocessing.ui \
    lineedit.ui \
    lrcedit.ui \
    lrcsearchneteasyform.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


contains(ANDROID_TARGET_ARCH,arm64-v8a) {
    ANDROID_PACKAGE_SOURCE_DIR = \
        $$PWD/android
}
