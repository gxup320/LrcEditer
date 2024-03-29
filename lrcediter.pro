QT       += core gui
QT       += multimedia
QT       += network
QT       += openglwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

LIBS += -lopengl32

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

TRANSLATIONS += "language/zh_CN.ts"

copydata.commands += $(COPY_DIR) $$shell_path($$PWD/language) $$shell_path($(OBJECTS_DIR)/language)
copydata.commands += & $(COPY) $$shell_path($$PWD/nextChar.json) $$shell_path($(OBJECTS_DIR))
win32 {
    copydata.commands += & $(COPY) $$shell_path($$PWD/ffmpeg_dir/ffmpeg_win64/ffmpeg.exe) $$shell_path($(OBJECTS_DIR))
}
unix:contains(QT_ARCH, x86_64){
    copydata.commands += & $(COPY) $$shell_path($$PWD/ffmpeg_dir/ffmpeg_linux64/ffmpeg) $$shell_path($(OBJECTS_DIR))
}

unix:contains(QT_ARCH, arm64){
    copydata.commands += & $(COPY) $$shell_path($$PWD/ffmpeg_dir/ffmpeg_linuxarm64/ffmpeg) $$shell_path($(OBJECTS_DIR))
}
unix {
    copydata.commands += & chmod +x $$shell_path($(OBJECTS_DIR)ffmpeg)
}
first.depends = $(first) copydata
export(first.depends)
export(copydata.commands)
QMAKE_EXTRA_TARGETS += first copydata

#translations_zh_CN.files = $$PWD/language/zh_CN.qm
#translations_zh_CN.path = language
#
#QMAKE_BUNDLE_DATA += translations_zh_CN

SOURCES += \
    batchprocessing.cpp \
    buffersizeedit.cpp \
    gaudioplayer.cpp \
    glrc.cpp \
    glrcline.cpp \
    glrcwindowgl.cpp \
    gpcmbar.cpp \
    gpcmbargl.cpp \
    gprogressbar.cpp \
    lineedit.cpp \
    lrcedit.cpp \
    lrcform.cpp \
    lrcqtextedit.cpp \
    lrcsearchneteasyform.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    batchprocessing.h \
    buffersizeedit.h \
    gaudioplayer.h \
    glrc.h \
    glrcline.h \
    glrcwindowgl.h \
    gpcmbar.h \
    gpcmbargl.h \
    gprogressbar.h \
    lineedit.h \
    lrcedit.h \
    lrcform.h \
    lrcqtextedit.h \
    lrcsearchneteasyform.h \
    mainwindow.h

FORMS += \
    batchprocessing.ui \
    buffersizeedit.ui \
    lineedit.ui \
    lrcedit.ui \
    lrcform.ui \
    lrcsearchneteasyform.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    ffmpeg_dir/ffmpeg_linux64/ffmpeg \
    ffmpeg_dir/ffmpeg_linuxarm64/ffmpeg \
    ffmpeg_dir/ffmpeg_win64/ffmpeg.exe \
    nextChar.json

