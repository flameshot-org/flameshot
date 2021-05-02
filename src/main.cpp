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
      QObject::tr("Path where the capture will be saved"),
      QStringLiteral("path"));
    CommandOption clipboardOption(
      { "c", "clipboard" }, QObject::tr("Save the capture to the clipboard"));
    CommandOption delayOption({ "d", "delay" },
                              QObject::tr("Delay time in milliseconds"),
                              QStringLiteral("milliseconds"));
    CommandOption filenameOption({ "f", "filename" },
                                 QObject::tr("Set the filename pattern"),
                                 QStringLiteral("pattern"));
    CommandOption trayOption({ "t", "trayicon" },
                             QObject::tr("Enable or disable the trayicon"),
                             QStringLiteral("bool"));
    CommandOption autostartOption(
      { "a", "autostart" },
      QObject::tr("Enable or disable run at startup"),
      QStringLiteral("bool"));
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
      QObject::tr("Define the screen to capture") + ",\n" +
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
    auto numericChecker = [](const QString& delayValue) -> bool {
        int value = delayValue.toInt();
        return value >= 0;
    };

    const QString pathErr =
      QObject::tr("Invalid path, it must be a real path in the system");
    auto pathChecker = [pathErr](const QString& pathValue) -> bool {
        bool res = QDir(pathValue).exists();
        if (!res) {
            SystemNotification().sendMessage(
              QObject::tr(pathErr.toLatin1().data()));
        }
        return res;
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
    parser.AddOptions(
      { pathOption, delayOption, rawImageOption, selectionOption },
      guiArgument);
    parser.AddOptions({ screenNumberOption,
                        clipboardOption,
                        pathOption,
                        delayOption,
                        rawImageOption },
                      screenArgument);
    parser.AddOptions(
      { pathOption, clipboardOption, delayOption, rawImageOption },
      fullArgument);
    parser.AddOptions({ autostartOption,
                        filenameOption,
                        trayOption,
                        showHelpOption,
                        mainColorOption,
                        contrastColorOption },
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
        QString pathValue = parser.value(pathOption);
        int delay = parser.value(delayOption).toInt();
        bool isRaw = parser.isSet(rawImageOption);
        bool isSelection = parser.isSet(selectionOption);
        DBusUtils dbusUtils;
        CaptureRequest req(CaptureRequest::GRAPHICAL_MODE, delay, pathValue);
        uint id = req.id();

        // Send message
        QDBusMessage m = QDBusMessage::createMethodCall(
          QStringLiteral("org.flameshot.Flameshot"),
          QStringLiteral("/"),
          QLatin1String(""),
          QStringLiteral("graphicCapture"));
        m << pathValue << delay << id;
        QDBusConnection sessionBus = QDBusConnection::sessionBus();
        dbusUtils.checkDBusConnection(sessionBus);
        sessionBus.call(m);

        if (isRaw) {
            dbusUtils.connectPrintCapture(sessionBus, id);
            return waitAfterConnecting(delay, app);
        } else if (isSelection) {
            dbusUtils.connectSelectionCapture(sessionBus, id);
            return waitAfterConnecting(delay, app);
        }
    } else if (parser.isSet(fullArgument)) { // FULL
        QString pathValue = parser.value(pathOption);
        int delay = parser.value(delayOption).toInt();
        bool toClipboard = parser.isSet(clipboardOption);
        bool isRaw = parser.isSet(rawImageOption);
        // Not a valid command
        if (!isRaw && !toClipboard && pathValue.isEmpty()) {
            QTextStream out(stdout);
            out << "Invalid format, set where to save the content with one of "
                << "the following flags:\n "
                << pathOption.dashedNames().join(QStringLiteral(", ")) << "\n "
                << rawImageOption.dashedNames().join(QStringLiteral(", "))
                << "\n "
                << clipboardOption.dashedNames().join(QStringLiteral(", "))
                << "\n\n";
            parser.parse(QStringList() << argv[0] << QStringLiteral("full")
                                       << QStringLiteral("-h"));
            goto finish;
        }

        CaptureRequest req(CaptureRequest::FULLSCREEN_MODE, delay, pathValue);
        if (toClipboard) {
            req.addTask(CaptureRequest::CLIPBOARD_SAVE_TASK);
        }
        if (!pathValue.isEmpty()) {
            req.addTask(CaptureRequest::FILESYSTEM_SAVE_TASK);
        }
        uint id = req.id();
        DBusUtils dbusUtils;

        // Send message
        QDBusMessage m = QDBusMessage::createMethodCall(
          QStringLiteral("org.flameshot.Flameshot"),
          QStringLiteral("/"),
          QLatin1String(""),
          QStringLiteral("fullScreen"));
        m << pathValue << toClipboard << delay << id;
        QDBusConnection sessionBus = QDBusConnection::sessionBus();
        dbusUtils.checkDBusConnection(sessionBus);
        sessionBus.call(m);

        if (isRaw) {
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
        int number =
          numberStr.startsWith(QLatin1String("-")) ? -1 : numberStr.toInt();
        QString pathValue = parser.value(pathOption);
        int delay = parser.value(delayOption).toInt();
        bool toClipboard = parser.isSet(clipboardOption);
        bool isRaw = parser.isSet(rawImageOption);
        // Not a valid command
        if (!isRaw && !toClipboard && pathValue.isEmpty()) {
            QTextStream out(stdout);
            out << "Invalid format, set where to save the content with one of "
                << "the following flags:\n "
                << pathOption.dashedNames().join(QStringLiteral(", ")) << "\n "
                << rawImageOption.dashedNames().join(QStringLiteral(", "))
                << "\n "
                << clipboardOption.dashedNames().join(QStringLiteral(", "))
                << "\n\n";
            parser.parse(QStringList() << argv[0] << QStringLiteral("screen")
                                       << QStringLiteral("-h"));
            goto finish;
        }

        CaptureRequest req(
          CaptureRequest::SCREEN_MODE, delay, pathValue, number);
        if (toClipboard) {
            req.addTask(CaptureRequest::CLIPBOARD_SAVE_TASK);
        }
        if (!pathValue.isEmpty()) {
            req.addTask(CaptureRequest::FILESYSTEM_SAVE_TASK);
        }
        uint id = req.id();
        DBusUtils dbusUtils;

        // Send message
        QDBusMessage m = QDBusMessage::createMethodCall(
          QStringLiteral("org.flameshot.Flameshot"),
          QStringLiteral("/"),
          QLatin1String(""),
          QStringLiteral("captureScreen"));
        m << number << pathValue << toClipboard << delay << id;
        QDBusConnection sessionBus = QDBusConnection::sessionBus();
        dbusUtils.checkDBusConnection(sessionBus);
        sessionBus.call(m);

        if (isRaw) {
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
        bool someFlagSet =
          (filename || tray || help || mainColor || contrastColor);
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
            config.setUIMainColor(parsedColor);
        }
        if (contrastColor) {
            QString colorCode = parser.value(contrastColorOption);
            QColor parsedColor(colorCode);
            config.setUIContrastColor(parsedColor);
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
