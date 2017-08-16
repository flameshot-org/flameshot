#-------------------------------------------------
#
# Project created by Dharkael 2017-04-21T00:42:49
#
#-------------------------------------------------

VERSION = $$system(git describe)
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

QT       += core gui
QT       += dbus

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG    += c++11
CONFIG    += link_pkgconfig

#CONFIG    += packaging   # Enables "make install" for packaging paths

TARGET = flameshot
TEMPLATE = app

TRANSLATIONS = translation/Internationalization_es.ts

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
include(src/third-party/singleapplication/singleapplication.pri)
include(src/third-party/Qt-Color-Widgets//color_widgets.pri)

DEFINES += QAPPLICATION_CLASS=QApplication

SOURCES += src/main.cpp\
    src/capture/widget/buttonhandler.cpp \
    src/infowindow.cpp \
    src/config/configwindow.cpp \
    src/capture/screenshot.cpp \
    src/capture/widget/capturewidget.cpp \
    src/capture/capturemodification.cpp \
    src/capture/widget/colorpicker.cpp \
    src/config/buttonlistview.cpp \
    src/config/uicoloreditor.cpp \
    src/config/geneneralconf.cpp \
    src/core/flameshotdbusadapter.cpp \
    src/core/controller.cpp \
    src/config/clickablelabel.cpp \
    src/config/filenameeditor.cpp \
    src/config/strftimechooserwidget.cpp \
    src/capture/tools/capturetool.cpp \
    src/capture/widget/capturebutton.cpp \
    src/capture/tools/penciltool.cpp \
    src/capture/tools/undotool.cpp \
    src/capture/tools/arrowtool.cpp \
    src/capture/tools/circletool.cpp \
    src/capture/tools/copytool.cpp \
    src/capture/tools/exittool.cpp \
    src/capture/tools/imguruploadertool.cpp \
    src/capture/tools/linetool.cpp \
    src/capture/tools/markertool.cpp \
    src/capture/tools/movetool.cpp \
    src/capture/tools/rectangletool.cpp \
    src/capture/tools/savetool.cpp \
    src/capture/tools/selectiontool.cpp \
    src/capture/tools/sizeindicatortool.cpp \
    src/capture/tools/toolfactory.cpp \
    src/utils/filenamehandler.cpp \
    src/utils/screengrabber.cpp \
    src/utils/confighandler.cpp \
    src/utils/systemnotification.cpp \
    src/cli/commandlineparser.cpp \
    src/cli/commandoption.cpp \
    src/cli/commandargument.cpp \
    src/capture/workers/screenshotsaver.cpp \
    src/capture/workers/imgur/imguruploader.cpp \
    src/capture/workers/graphicalscreenshotsaver.cpp \
    src/capture/workers/imgur/loadspinner.cpp \
    src/capture/workers/imgur/imagelabel.cpp \
    src/capture/workers/imgur/notificationwidget.cpp

HEADERS  += \
    src/capture/widget/buttonhandler.h \
    src/infowindow.h \
    src/config/configwindow.h \
    src/capture/screenshot.h \
    src/capture/widget/capturewidget.h \
    src/capture/capturemodification.h \
    src/capture/widget/colorpicker.h \
    src/config/buttonlistview.h \
    src/config/uicoloreditor.h \
    src/config/geneneralconf.h \
    src/core/flameshotdbusadapter.h \
    src/config/clickablelabel.h \
    src/config/filenameeditor.h \
    src/utils/filenamehandler.h \
    src/config/strftimechooserwidget.h \
    src/utils/screengrabber.h \
    src/capture/tools/capturetool.h \
    src/capture/widget/capturebutton.h \
    src/capture/tools/penciltool.h \
    src/capture/tools/undotool.h \
    src/capture/tools/arrowtool.h \
    src/capture/tools/circletool.h \
    src/capture/tools/copytool.h \
    src/capture/tools/exittool.h \
    src/capture/tools/imguruploadertool.h \
    src/capture/tools/linetool.h \
    src/capture/tools/markertool.h \
    src/capture/tools/movetool.h \
    src/capture/tools/rectangletool.h \
    src/capture/tools/savetool.h \
    src/capture/tools/selectiontool.h \
    src/capture/tools/sizeindicatortool.h \
    src/capture/tools/toolfactory.h \
    src/utils/confighandler.h \
    src/core/controller.h \
    src/utils/systemnotification.h \
    src/cli/commandlineparser.h \
    src/cli/commandoption.h \
    src/cli/commandargument.h \
    src/capture/workers/screenshotsaver.h \
    src/capture/workers/imgur/imguruploader.h \
    src/capture/workers/graphicalscreenshotsaver.h \
    src/capture/workers/imgur/loadspinner.h \
    src/capture/workers/imgur/imagelabel.h \
    src/capture/workers/imgur/notificationwidget.h

RESOURCES += \
    graphics.qrc

# installs
unix: {
    packaging {
        USRPATH = /usr
    } else {
        USRPATH = /usr/local
    }

    target.path = $${BASEDIR}$${USRPATH}/bin/

    qmfile.path = $${BASEDIR}/usr/share/flameshot/translations/
    qmfile.files = translation/Internationalization_es.qm

    dbus.path = $${BASEDIR}/usr/share/dbus-1/interfaces/
    dbus.files = dbus/org.dharkael.Flameshot.xml
    
    icon.path = $${BASEDIR}$${USRPATH}/share/icons/
    icon.files = img/flameshot.png
    
    desktopentry.path = $${BASEDIR}$${USRPATH}/share/applications
    desktopentryinit.path = $${BASEDIR}$${USRPATH}/share/applications
    servicedbus.path = $${BASEDIR}/usr/share/dbus-1/services/

    packaging {
        desktopentry.files = docs/desktopEntry/package/flameshot.desktop
        desktopentryinit.files = docs/desktopEntry/package/flameshot-init.desktop
        servicedbus.files = dbus/package/org.dharkael.Flameshot.service
    } else {
        desktopentry.files = docs/desktopEntry/make/flameshot.desktop
        desktopentryinit.files = docs/desktopEntry/make/flameshot-init.desktop
        servicedbus.files = dbus/make/org.dharkael.Flameshot.service
    }

    INSTALLS += target \
        icon \
        desktopentry \
        desktopentryinit \
        qmfile \
        servicedbus \
        dbus
}

