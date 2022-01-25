#-------------------------------------------------
#
# Project created by QtCreator 2016-02-05T22:01:03
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = HotkeyTest
TEMPLATE = app

include(../qhotkey.pri)

SOURCES += main.cpp\
        hottestwidget.cpp

HEADERS  += hottestwidget.h

FORMS    += hottestwidget.ui
