#-------------------------------------------------
#
# Project created by QtCreator 2016-07-30T21:36:19
#
#-------------------------------------------------

QT       += core gui

#RC_ICONS += linkupgame.ico

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = LinkUp
TEMPLATE = app


SOURCES += main.cpp\
        gamewidget.cpp \
    imagebtn.cpp

HEADERS  += gamewidget.h \
    imagebtn.h \
    linkup_common.h

RESOURCES += \
    images.qrc
