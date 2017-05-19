#-------------------------------------------------
#
# Project created by Dharkael 2017-04-21T00:42:49
#
#-------------------------------------------------


QT       += core gui
QT       += x11extras

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG    += c++11
CONFIG    += link_pkgconfig
PKGCONFIG += x11

TARGET = flameshot
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
include(singleapplication/singleapplication.pri)
include(Qt-Color-Widgets//color_widgets.pri)

DEFINES += QAPPLICATION_CLASS=QApplication

SOURCES += main.cpp\
    nativeeventfilter.cpp \
    controller.cpp \
    capture/button.cpp \
    capture/buttonhandler.cpp \
    infowindow.cpp \
    config/configwindow.cpp \
    capture/screenshot.cpp \
    capture/capturewidget.cpp \
    capture/capturemodification.cpp \
    capture/colorpicker.cpp \
    config/buttonlistview.cpp

HEADERS  += \
    nativeeventfilter.h \
    controller.h \
    capture/button.h \
    capture/buttonhandler.h \
    infowindow.h \
    config/configwindow.h \
    capture/screenshot.h \
    capture/capturewidget.h \
    capture/capturemodification.h \
    capture/colorpicker.h \
    config/buttonlistview.h

RESOURCES += \
    graphics.qrc
