#-------------------------------------------------
#
# Project created by Dharkael 2017-04-21T00:42:49
#
#-------------------------------------------------

win32:LIBS += -luser32 -lshell32

TAG_VERSION = $$system(git --git-dir $$PWD/.git --work-tree $$PWD describe --always --tags)
DEFINES += APP_VERSION=\\\"$$TAG_VERSION\\\"

QT  += core gui widgets network

unix:!macx {
    QT  += dbus
}

CONFIG += c++11 link_pkgconfig

#CONFIG += packaging   # Enables "make install" for packaging paths

TARGET = flameshot
TEMPLATE = app

win32:RC_ICONS += img/flameshot.ico

#release: DESTDIR = build/release
#debug:   DESTDIR = build/debug

#OBJECTS_DIR = $$DESTDIR/.obj
#MOC_DIR = $$DESTDIR/.moc
#RCC_DIR = $$DESTDIR/.qrc
#UI_DIR = $$DESTDIR/.ui

TRANSLATIONS = translations/Internationalization_es.ts \
    translations/Internationalization_ca.ts \
    translations/Internationalization_ru.ts \
    translations/Internationalization_zh_CN.ts \
    translations/Internationalization_zh_TW.ts \
    translations/Internationalization_tr.ts \
    translations/Internationalization_ka.ts \
    translations/Internationalization_fr.ts \
    translations/Internationalization_pl.ts

# Generate translations in build
TRANSLATIONS_FILES =

qtPrepareTool(LRELEASE, lrelease)
for(tsfile, TRANSLATIONS) {
    qmfile = $$shadowed($$tsfile)
    qmfile ~= s,.ts$,.qm,
    qmdir = $$dirname(qmfile)
    !exists($$qmdir) {
        mkpath($$qmdir)|error("Aborting.")
    }
    command = $$LRELEASE -removeidentical $$tsfile -qm $$qmfile
    system($$command)|error("Failed to run: $$command")
    TRANSLATIONS_FILES += $$qmfile
}

DEFINES += QT_DEPRECATED_WARNINGS

include(src/third-party/singleapplication/singleapplication.pri)
include(src/third-party/Qt-Color-Widgets//color_widgets.pri)

DEFINES += QAPPLICATION_CLASS=QApplication

SOURCES += src/main.cpp \
    src/capture/widgets/buttonhandler.cpp \
    src/infowindow.cpp \
    src/config/configwindow.cpp \
    src/capture/screenshot.cpp \
    src/capture/widgets/capturewidget.cpp \
    src/capture/capturemodification.cpp \
    src/capture/widgets/colorpicker.cpp \
    src/config/buttonlistview.cpp \
    src/config/uicoloreditor.cpp \
    src/config/geneneralconf.cpp \
    src/core/controller.cpp \
    src/config/clickablelabel.cpp \
    src/config/filenameeditor.cpp \
    src/config/strftimechooserwidget.cpp \
    src/capture/tools/capturetool.cpp \
    src/capture/widgets/capturebutton.cpp \
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
    src/widgets/loadspinner.cpp \
    src/widgets/imagelabel.cpp \
    src/widgets/notificationwidget.cpp \
    src/core/resourceexporter.cpp \
    src/capture/widgets/notifierbox.cpp \
    src/utils/desktopinfo.cpp \
    src/capture/workers/launcher/applauncherwidget.cpp \
    src/capture/tools/applauncher.cpp \
    src/utils/desktopfileparse.cpp \
    src/capture/workers/launcher/launcheritemdelegate.cpp \
    src/capture/tools/blurtool.cpp \
    src/capture/workers/launcher/terminallauncher.cpp \
    src/config/visualseditor.cpp \
    src/config/extendedslider.cpp \
    src/capture/workers/launcher/openwithprogram.cpp \
    src/utils/pathinfo.cpp

HEADERS  += src/capture/widgets/buttonhandler.h \
    src/infowindow.h \
    src/config/configwindow.h \
    src/capture/screenshot.h \
    src/capture/widgets/capturewidget.h \
    src/capture/capturemodification.h \
    src/capture/widgets/colorpicker.h \
    src/config/buttonlistview.h \
    src/config/uicoloreditor.h \
    src/config/geneneralconf.h \
    src/config/clickablelabel.h \
    src/config/filenameeditor.h \
    src/utils/filenamehandler.h \
    src/config/strftimechooserwidget.h \
    src/utils/screengrabber.h \
    src/capture/tools/capturetool.h \
    src/capture/widgets/capturebutton.h \
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
    src/widgets/loadspinner.h \
    src/widgets/imagelabel.h \
    src/widgets/notificationwidget.h \
    src/core/resourceexporter.h \
    src/capture/widgets/notifierbox.h \
    src/utils/desktopinfo.h \
    src/capture/workers/launcher/applauncherwidget.h \
    src/capture/tools/applauncher.h \
    src/utils/desktopfileparse.h \
    src/capture/workers/launcher/launcheritemdelegate.h \
    src/capture/tools/blurtool.h \
    src/capture/workers/launcher/terminallauncher.h \
    src/config/visualseditor.h \
    src/config/extendedslider.h \
    src/capture/workers/launcher/openwithprogram.h \
    src/utils/pathinfo.h

unix:!macx {
    SOURCES += src/core/flameshotdbusadapter.cpp \
        src/utils/dbusutils.cpp

    HEADERS  += src/core/flameshotdbusadapter.h \
        src/utils/dbusutils.h
}

win32 {
    SOURCES += src/core/globalshortcutfilter.cpp

    HEADERS  += src/core/globalshortcutfilter.h
}

RESOURCES += \
    graphics.qrc

# installs
unix:!macx {
    isEmpty(PREFIX) {
      packaging {
        PREFIX = /usr
      } else {
        PREFIX = /usr/local
      }
    }

    DEFINES += APP_PREFIX=\\\"$$PREFIX\\\"

    target.path = $${BASEDIR}$${PREFIX}/bin/

    qmfile.path = $${BASEDIR}$${PREFIX}/share/flameshot/translations/
    qmfile.files = $${TRANSLATIONS_FILES}

    dbus.path = $${BASEDIR}$${PREFIX}/share/dbus-1/interfaces/
    dbus.files = dbus/org.dharkael.Flameshot.xml
    
    icon.path = $${BASEDIR}$${PREFIX}/share/icons/
    icon.files = img/flameshot.png

    completion.path = $${BASEDIR}$${PREFIX}/share/bash-completion/completions/
    completion.files = docs/bash-completion/flameshot
    
    appdata.path = $${BASEDIR}$${PREFIX}/share/metainfo/
    appdata.files = docs/appdata/flameshot.appdata.xml

    desktopentry.path = $${BASEDIR}$${PREFIX}/share/applications
    desktopentry.files = docs/desktopEntry/package/flameshot.desktop

    desktopentryinit.path = $${BASEDIR}$${PREFIX}/share/applications
    desktopentryinit.files = docs/desktopEntry/package/flameshot-init.desktop

    desktopentryconfig.path = $${BASEDIR}$${PREFIX}/share/applications
    desktopentryconfig.files = docs/desktopEntry/package/flameshot-config.desktop

    servicedbus.path = $${BASEDIR}$${PREFIX}/share/dbus-1/services/
    packaging {
        servicedbus.files = dbus/package/org.dharkael.Flameshot.service
    } else {
        servicedbus.files = dbus/make/org.dharkael.Flameshot.service
    }

    INSTALLS += target \
        icon \
        desktopentry \
        desktopentryinit \
        desktopentryconfig \
        qmfile \
        servicedbus \
        dbus \
        completion \
        appdata
}

