#-------------------------------------------------
#
# Project created by QtCreator 2012-07-02T17:47:18
#
#-------------------------------------------------

QT       += core gui

TARGET = fatx-explorer
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    xtafdisk.cpp \
    xtafpart.cpp \
    xtaffilesystemmodel.cpp \
    xtaffsys.cpp

HEADERS  += mainwindow.h \
    xtafdisk.h \
    xtafpart.h \
    xtaffilesystemmodel.h \
    xtaffsys.h

FORMS    += mainwindow.ui
