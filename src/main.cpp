// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#ifndef USE_EXTERNAL_SINGLEAPPLICATION
#include "singleapplication.h"
#else
#include "QtSolutions/qtsingleapplication.h"
#endif

#include "src/cli/commandlineparser.h"
#include "src/config/styleoverride.h"
#include "src/core/capturerequest.h"
#include "src/core/controller.h"
#include "src/utils/confighandler.h"
#include "src/utils/filenamehandler.h"
#include "src/utils/pathinfo.h"
#include "src/utils/systemnotification.h"
#include "src/utils/valuehandler.h"
#include <QApplication>
#include <QDir>
#include <QLibraryInfo>
#include <QTextStream>
#include <QTimer>
#include <QTranslator>

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#include "spdlog/spdlog.h"

#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
#include "src/core/flameshotdbusadapter.h"
#include "src/utils/dbusutils.h"
#include <QDBusConnection>
#include <QDBusMessage>
#include <desktopinfo.h>
#endif

int waitAfterConnecting(int delay, QCoreApplication& app)
{
    QTimer t;
    t.setInterval(delay + 1000 * 60 * 15); // 15 minutes timeout
    QObject::connect(&t, &QTimer::timeout, qApp, &QCoreApplication::quit);
    t.start();
    // wait
    return app.exec();
}

#ifdef Q_OS_LINUX
// source: https://github.com/ksnip/ksnip/issues/416
void wayland_hacks()
{
    // Workaround to https://github.com/ksnip/ksnip/issues/416
    DesktopInfo info;
    if (info.windowManager() == DesktopInfo::GNOME) {
        qputenv("QT_QPA_PLATFORM", "xcb");
    }
}
#endif

int main(int argc, char* argv[])
{
#ifdef Q_OS_LINUX
    wayland_hacks();
#endif
    spdlog::set_level(spdlog::level::debug); // Set global log level to debug
    spdlog::set_pattern("[source %s] [function %!] [line %#] %v");

    // required for the button serialization
    // TODO: change to QVector in v1.0
    qRegisterMetaTypeStreamOperators<QList<int>>("QList<int>");
    qApp->setApplicationVersion(static_cast<QString>(APP_VERSION));

    // no arguments, just launch Flameshot
    if (argc == 1) {
#ifndef USE_EXTERNAL_SINGLEAPPLICATION
        SingleApplication app(argc, argv);
#else
        QtSingleApplication app(argc, argv);
#endif
        QApplication::setStyle(new StyleOverride);

        QTranslator translator, qtTranslator;
        QStringList trPaths = PathInfo::translationsPaths();

        for (const QString& path : trPaths) {
            bool match = translator.load(QLocale(),
                                         QStringLiteral("Internationalization"),
                                         QStringLiteral("_"),
                                         path);
            if (match) {
                break;
            }
        }

        qtTranslator.load(
          QLocale::system(),
          "qt",
          "_",
          QLibraryInfo::location(QLibraryInfo::TranslationsPath));

        app.installTranslator(&translator);
        app.installTranslator(&qtTranslator);
        app.setAttribute(Qt::AA_DontCreateNativeWidgetSiblings, true);
        app.setApplicationName(QStringLiteral("flameshot"));
        app.setOrganizationName(QStringLiteral("flameshot"));

        auto c = Controller::getInstance();
#if not(defined(Q_OS_MACOS) || defined(Q_OS_WIN))
        new FlameshotDBusAdapter(c);
        QDBusConnection dbus = QDBusConnection::sessionBus();
        if (!dbus.isConnected()) {
            SystemNotification().sendMessage(
              QObject::tr("Unable to connect via DBus"));
        }
        dbus.registerObject(QStringLiteral("/"), c);
        dbus.registerService(QStringLiteral("org.flameshot.Flameshot"));
#endif
        // Exporting captures must be connected after the dbus interface
        // or the dbus signal gets blocked until we end the exports.
        c->enableExports();
        return app.exec();
    }

#if not(defined(Q_OS_MACOS) || defined(Q_OS_WIN))
    /*--------------|
     * CLI parsing  |
     * ------------*/
    QCoreApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("flameshot"));
    app.setOrganizationName(QStringLiteral("flameshot"));
    app.setApplicationVersion(qApp->applicationVersion());
    CommandLineParser parser;
    // Add description
    parser.setDescription(
      QObject::tr("Powerful yet simple to use screenshot software."));
    parser.setGeneralErrorMessage(QObject::tr("See") + " flameshot --help.");
    // Arguments
    CommandArgument fullArgument(QStringLiteral("full"),
                                 QObject::tr("Capture the entire desktop."));
    CommandArgument launcherArgument(QStringLiteral("launcher"),
                                     QObject::tr("Open the capture launcher."));
    CommandArgument guiArgument(
      QStringLiteral("gui"),
      QObject::tr("Start a manual capture in GUI mode."));
    CommandArgument configArgument(QStringLiteral("config"),
                                   QObject::tr("Configure") + " flameshot.");
    CommandArgument screenArgument(QStringLiteral("screen"),
                                   QObject::tr("Capture a single screen."));

    // Options
    CommandOption pathOption(
      { "p", "path" },
      QObject::tr("Existing directory or new file to save to"),
      QStringLiteral("path"));
    CommandOption clipboardOption(
      { "c", "clipboard" }, QObject::tr("Save the capture to the clipboard"));
    CommandOption pinOption("pin",
                            QObject::tr("Pin the capture to the screen"));
    CommandOption uploadOption({ "u", "upload" },
                               QObject::tr("Upload screenshot"));
    CommandOption delayOption({ "d", "delay" },
                              QObject::tr("Delay time in milliseconds"),
                              QStringLiteral("milliseconds"));
    CommandOption regionOption("region",
                               QObject::tr("Screenshot region to select"),
                               QStringLiteral("WxH+X+Y or string"));
    CommandOption filenameOption({ "f", "filename" },
                                 QObject::tr("Set the filename pattern"),
                                 QStringLiteral("pattern"));
    CommandOption acceptOnSelectOption(
      { "s", "accept-on-select" },
      QObject::tr("Accept capture as soon as a selection is made"));
    CommandOption trayOption({ "t", "trayicon" },
                             QObject::tr("Enable or disable the trayicon"),
                             QStringLiteral("bool"));
    CommandOption autostartOption(
      { "a", "autostart" },
      QObject::tr("Enable or disable run at startup"),
      QStringLiteral("bool"));
    CommandOption checkOption(
      "check", QObject::tr("Check the configuration for errors"));
    CommandOption showHelpOption(
      { "s", "showhelp" },
      QObject::tr("Show the help message in the capture mode"),
      QStringLiteral("bool"));
    CommandOption mainColorOption({ "m", "maincolor" },
                                  QObject::tr("Define the main UI color"),
                                  QStringLiteral("color-code"));
    CommandOption contrastColorOption(
      { "k", "contrastcolor" },
      QObject::tr("Define the contrast UI color"),
      QStringLiteral("color-code"));
    CommandOption rawImageOption({ "r", "raw" },
                                 QObject::tr("Print raw PNG capture"));
    CommandOption selectionOption(
      { "g", "print-geometry" },
      QObject::tr("Print geometry of the selection in the format W H X Y. Does "
                  "nothing if raw is specified"));
    CommandOption screenNumberOption(
      { "n", "number" },
      QObject::tr("Define the screen to capture (starting from 0)") + ",\n" +
        QObject::tr("default: screen containing the cursor"),
      QObject::tr("Screen number"),
      QStringLiteral("-1"));

    // Add checkers
    auto colorChecker = [](const QString& colorCode) -> bool {
        QColor parsedColor(colorCode);
        return parsedColor.isValid() && parsedColor.alphaF() == 1.0;
    };
    QString colorErr =
      QObject::tr("Invalid color, "
                  "this flag supports the following formats:\n"
                  "- #RGB (each of R, G, and B is a single hex digit)\n"
                  "- #RRGGBB\n- #RRRGGGBBB\n"
                  "- #RRRRGGGGBBBB\n"
                  "- Named colors like 'blue' or 'red'\n"
                  "You may need to escape the '#' sign as in '\\#FFF'");

    const QString delayErr =
      QObject::tr("Invalid delay, it must be higher than 0");
    const QString numberErr =
      QObject::tr("Invalid screen number, it must be non negative");
    const QString regionErr = QObject::tr(
      "Invalid region, use 'WxH+X+Y' or 'all' or 'screen0/screen1/...'.");
    auto numericChecker = [](const QString& delayValue) -> bool {
        int value = delayValue.toInt();
        return value >= 0;
    };
    auto regionChecker = [](const QString& region) -> bool {
        Region valueHandler;
        return valueHandler.check(region);
    };

    const QString pathErr =
      QObject::tr("Invalid path, must be an existing directory or a new file "
                  "in an existing directory");
    auto pathChecker = [pathErr](const QString& pathValue) -> bool {
        QFileInfo fileInfo(pathValue);
        if (fileInfo.isDir() || fileInfo.dir().exists()) {
            return true;
        } else {
            SystemNotification().sendMessage(
              QObject::tr(pathErr.toLatin1().data()));
            return false;
        }
    };

    const QString booleanErr =
      QObject::tr("Invalid value, it must be defined as 'true' or 'false'");
    auto booleanChecker = [](const QString& value) -> bool {
        return value == QLatin1String("true") ||
               value == QLatin1String("false");
    };

    contrastColorOption.addChecker(colorChecker, colorErr);
    mainColorOption.addChecker(colorChecker, colorErr);
    delayOption.addChecker(numericChecker, delayErr);
    regionOption.addChecker(regionChecker, regionErr);
    pathOption.addChecker(pathChecker, pathErr);
    trayOption.addChecker(booleanChecker, booleanErr);
    autostartOption.addChecker(booleanChecker, booleanErr);
    showHelpOption.addChecker(booleanChecker, booleanErr);
    screenNumberOption.addChecker(numericChecker, numberErr);

    // Relationships
    parser.AddArgument(guiArgument);
    parser.AddArgument(screenArgument);
    parser.AddArgument(fullArgument);
    parser.AddArgument(launcherArgument);
    parser.AddArgument(configArgument);
    auto helpOption = parser.addHelpOption();
    auto versionOption = parser.addVersionOption();
    parser.AddOptions({ pathOption,
                        clipboardOption,
                        delayOption,
                        regionOption,
                        rawImageOption,
                        selectionOption,
                        uploadOption,
                        pinOption,
                        acceptOnSelectOption },
                      guiArgument);
    parser.AddOptions({ screenNumberOption,
                        clipboardOption,
                        pathOption,
                        delayOption,
                        regionOption,
                        rawImageOption,
                        uploadOption,
                        pinOption },
                      screenArgument);
    parser.AddOptions({ pathOption,
                        clipboardOption,
                        delayOption,
                        regionOption,
                        rawImageOption,
                        uploadOption },
                      fullArgument);
    parser.AddOptions({ autostartOption,
                        filenameOption,
                        trayOption,
                        showHelpOption,
                        mainColorOption,
                        contrastColorOption,
                        checkOption },
                      configArgument);
    // Parse
    if (!parser.parse(app.arguments())) {
        goto finish;
    }

    // PROCESS DATA
    //--------------
    if (parser.isSet(helpOption) || parser.isSet(versionOption)) {
    } else if (parser.isSet(launcherArgument)) { // LAUNCHER
        QDBusMessage m = QDBusMessage::createMethodCall(
          QStringLiteral("org.flameshot.Flameshot"),
          QStringLiteral("/"),
          QLatin1String(""),
          QStringLiteral("openLauncher"));
        QDBusConnection sessionBus = QDBusConnection::sessionBus();
        if (!sessionBus.isConnected()) {
            SystemNotification().sendMessage(
              QObject::tr("Unable to connect via DBus"));
        }
        sessionBus.call(m);
    } else if (parser.isSet(guiArgument)) { // GUI
        // Option values
        QString path = parser.value(pathOption);
        if (!path.isEmpty()) {
            path = QDir(path).absolutePath();
        }
        int delay = parser.value(delayOption).toInt();
        QString region = parser.value(regionOption);
        bool clipboard = parser.isSet(clipboardOption);
        bool raw = parser.isSet(rawImageOption);
        bool printGeometry = parser.isSet(selectionOption);
        bool pin = parser.isSet(pinOption);
        bool upload = parser.isSet(uploadOption);
        bool acceptOnSelect = parser.isSet(acceptOnSelectOption);
        DBusUtils dbusUtils;
        CaptureRequest req(CaptureRequest::GRAPHICAL_MODE, delay, path);
        if (!region.isEmpty()) {
            req.setInitialSelection(Region().value(region).toRect());
        }
        if (clipboard) {
            req.addTask(CaptureRequest::COPY);
        }
        if (raw) {
            req.addTask(CaptureRequest::PRINT_RAW);
        }
        if (!path.isEmpty()) {
            req.addSaveTask(path);
        }
        if (printGeometry) {
            req.addTask(CaptureRequest::PRINT_GEOMETRY);
        }
        if (pin) {
            req.addTask(CaptureRequest::PIN);
        }
        if (upload) {
            req.addTask(CaptureRequest::UPLOAD);
        }
        if (acceptOnSelect) {
            req.addTask(CaptureRequest::ACCEPT_ON_SELECT);
            if (!clipboard && !raw && path.isEmpty() && !printGeometry &&
                !pin && !upload) {
                req.addSaveTask();
            }
        }
        uint id = req.id();
        req.setStaticID(id);

        // Send message
        QDBusMessage m = QDBusMessage::createMethodCall(
          QStringLiteral("org.flameshot.Flameshot"),
          QStringLiteral("/"),
          QLatin1String(""),
          QStringLiteral("requestCapture"));
        m << req.serialize();
        QDBusConnection sessionBus = QDBusConnection::sessionBus();
        dbusUtils.checkDBusConnection(sessionBus);
        sessionBus.call(m);

        if (raw) {
            dbusUtils.connectPrintCapture(sessionBus, id);
            return waitAfterConnecting(delay, app);
        } else if (printGeometry) {
            dbusUtils.connectSelectionCapture(sessionBus, id);
            return waitAfterConnecting(delay, app);
        }
    } else if (parser.isSet(fullArgument)) { // FULL
        // Option values
        QString path = parser.value(pathOption);
        if (!path.isEmpty()) {
            path = QDir(path).absolutePath();
        }
        int delay = parser.value(delayOption).toInt();
        QString region = parser.value(regionOption);
        bool clipboard = parser.isSet(clipboardOption);
        bool raw = parser.isSet(rawImageOption);
        bool upload = parser.isSet(uploadOption);
        // Not a valid command

        CaptureRequest req(CaptureRequest::FULLSCREEN_MODE, delay);
        if (!region.isEmpty()) {
            req.setInitialSelection(Region().value(region).toRect());
        }
        if (clipboard) {
            req.addTask(CaptureRequest::COPY);
        }
        if (!path.isEmpty()) {
            req.addSaveTask(path);
        }
        if (raw) {
            req.addTask(CaptureRequest::PRINT_RAW);
        }
        if (upload) {
            req.addTask(CaptureRequest::UPLOAD);
        }
        if (!clipboard && path.isEmpty() && !raw && !upload) {
            req.addSaveTask();
        }
        uint id = req.id();
        req.setStaticID(id);
        DBusUtils dbusUtils;

        // Send message
        QDBusMessage m = QDBusMessage::createMethodCall(
          QStringLiteral("org.flameshot.Flameshot"),
          QStringLiteral("/"),
          QLatin1String(""),
          QStringLiteral("requestCapture"));
        m << req.serialize();
        QDBusConnection sessionBus = QDBusConnection::sessionBus();
        dbusUtils.checkDBusConnection(sessionBus);
        sessionBus.call(m);

        if (raw) {
            dbusUtils.connectPrintCapture(sessionBus, id);
            // timeout just in case
            QTimer t;
            t.setInterval(delay + 2000);
            QObject::connect(
              &t, &QTimer::timeout, qApp, &QCoreApplication::quit);
            t.start();
            // wait
            return app.exec();
        }
    } else if (parser.isSet(screenArgument)) { // SCREEN
        QString numberStr = parser.value(screenNumberOption);
        // Option values
        int number =
          numberStr.startsWith(QLatin1String("-")) ? -1 : numberStr.toInt();
        QString path = parser.value(pathOption);
        if (!path.isEmpty()) {
            path = QDir(path).absolutePath();
        }
        int delay = parser.value(delayOption).toInt();
        QString region = parser.value(regionOption);
        bool clipboard = parser.isSet(clipboardOption);
        bool raw = parser.isSet(rawImageOption);
        bool pin = parser.isSet(pinOption);
        bool upload = parser.isSet(uploadOption);

        CaptureRequest req(CaptureRequest::SCREEN_MODE, delay, number);
        if (!region.isEmpty()) {
            if (region.startsWith("screen")) {
                // TODO use abstract logger
                QTextStream(stderr) << "The 'screen' command does not support "
                                       "'--region screen<N>'.\n"
                                       "See flameshot --help.\n";
                exit(1);
            }
            req.setInitialSelection(Region().value(region).toRect());
        }
        if (clipboard) {
            req.addTask(CaptureRequest::COPY);
        }
        if (raw) {
            req.addTask(CaptureRequest::PRINT_RAW);
        }
        if (!path.isEmpty()) {
            req.addSaveTask(path);
        }
        if (pin) {
            req.addTask(CaptureRequest::PIN);
        }
        if (upload) {
            req.addTask(CaptureRequest::UPLOAD);
        }

        if (!clipboard && !raw && path.isEmpty() && !pin && !upload) {
            req.addSaveTask();
        }

        uint id = req.id();
        req.setStaticID(id);
        DBusUtils dbusUtils;

        // Send message
        QDBusMessage m = QDBusMessage::createMethodCall(
          QStringLiteral("org.flameshot.Flameshot"),
          QStringLiteral("/"),
          QLatin1String(""),
          QStringLiteral("requestCapture"));
        m << req.serialize();
        QDBusConnection sessionBus = QDBusConnection::sessionBus();
        dbusUtils.checkDBusConnection(sessionBus);
        sessionBus.call(m);

        if (raw) {
            dbusUtils.connectPrintCapture(sessionBus, id);
            // timeout just in case
            QTimer t;
            t.setInterval(delay + 2000);
            QObject::connect(
              &t, &QTimer::timeout, qApp, &QCoreApplication::quit);
            t.start();
            // wait
            return app.exec();
        }
    } else if (parser.isSet(configArgument)) { // CONFIG
        bool autostart = parser.isSet(autostartOption);
        bool filename = parser.isSet(filenameOption);
        bool tray = parser.isSet(trayOption);
        bool help = parser.isSet(showHelpOption);
        bool mainColor = parser.isSet(mainColorOption);
        bool contrastColor = parser.isSet(contrastColorOption);
        bool check = parser.isSet(checkOption);
        bool someFlagSet =
          (filename || tray || help || mainColor || contrastColor || check);
        if (check) {
            QTextStream stream(stderr);
            bool ok = ConfigHandler(true).checkForErrors(&stream);
            if (ok) {
                stream << QStringLiteral("No errors detected.\n");
                goto finish;
            } else {
                return 1;
            }
        }
        ConfigHandler config;
        if (autostart) {
            QDBusMessage m = QDBusMessage::createMethodCall(
              QStringLiteral("org.flameshot.Flameshot"),
              QStringLiteral("/"),
              QLatin1String(""),
              QStringLiteral("autostartEnabled"));
            if (parser.value(autostartOption) == QLatin1String("false")) {
                m << false;
            } else if (parser.value(autostartOption) == QLatin1String("true")) {
                m << true;
            }
            QDBusConnection sessionBus = QDBusConnection::sessionBus();
            if (!sessionBus.isConnected()) {
                SystemNotification().sendMessage(
                  QObject::tr("Unable to connect via DBus"));
            }
            sessionBus.call(m);
        }
        if (filename) {
            QString newFilename(parser.value(filenameOption));
            config.setFilenamePattern(newFilename);
            FileNameHandler fh;
            QTextStream(stdout)
              << QStringLiteral("The new pattern is '%1'\n"
                                "Parsed pattern example: %2\n")
                   .arg(newFilename)
                   .arg(fh.parsedPattern());
        }
        if (tray) {
            QDBusMessage m = QDBusMessage::createMethodCall(
              QStringLiteral("org.flameshot.Flameshot"),
              QStringLiteral("/"),
              QLatin1String(""),
              QStringLiteral("trayIconEnabled"));
            if (parser.value(trayOption) == QLatin1String("false")) {
                m << false;
            } else if (parser.value(trayOption) == QLatin1String("true")) {
                m << true;
            }
            QDBusConnection sessionBus = QDBusConnection::sessionBus();
            if (!sessionBus.isConnected()) {
                SystemNotification().sendMessage(
                  QObject::tr("Unable to connect via DBus"));
            }
            sessionBus.call(m);
        }
        if (help) {
            if (parser.value(showHelpOption) == QLatin1String("false")) {
                config.setShowHelp(false);
            } else if (parser.value(showHelpOption) == QLatin1String("true")) {
                config.setShowHelp(true);
            }
        }
        if (mainColor) {
            QString colorCode = parser.value(mainColorOption);
            QColor parsedColor(colorCode);
            config.setUiColor(parsedColor);
        }
        if (contrastColor) {
            QString colorCode = parser.value(contrastColorOption);
            QColor parsedColor(colorCode);
            config.setContrastUiColor(parsedColor);
        }

        // Open gui when no options
        if (!someFlagSet) {
            QDBusMessage m = QDBusMessage::createMethodCall(
              QStringLiteral("org.flameshot.Flameshot"),
              QStringLiteral("/"),
              QLatin1String(""),
              QStringLiteral("openConfig"));
            QDBusConnection sessionBus = QDBusConnection::sessionBus();
            if (!sessionBus.isConnected()) {
                SystemNotification().sendMessage(
                  QObject::tr("Unable to connect via DBus"));
            }
            sessionBus.call(m);
        }
    }
finish:

#endif
    return 0;
}
