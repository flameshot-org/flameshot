// Copyright(c) 2017-2018 Alejandro Sirgo Rica & Contributors
//
// This file is part of Flameshot.
//
//     Flameshot is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     Flameshot is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with Flameshot.  If not, see <http://www.gnu.org/licenses/>.

#include "src/core/controller.h"
#include "singleapplication.h"
#include "src/utils/filenamehandler.h"
#include "src/utils/confighandler.h"
#include "src/cli/commandlineparser.h"
#include "src/utils/systemnotification.h"
#include "src/utils/pathinfo.h"
#include <QApplication>
#include <QTranslator>
#include <QTextStream>
#include <QTimer>
#include <QDir>

#ifdef Q_OS_LINUX
#include "src/core/flameshotdbusadapter.h"
#include "src/utils/dbusutils.h"
#include <QDBusMessage>
#include <QDBusConnection>
#endif

int main(int argc, char *argv[]) {
    // required for the button serialization
    // TODO: change to QVector in v1.0
    qRegisterMetaTypeStreamOperators<QList<int> >("QList<int>");
    qApp->setApplicationVersion(static_cast<QString>(APP_VERSION));

    // no arguments, just launch Flameshot
    if (argc == 1) {
        SingleApplication app(argc, argv);

        QTranslator translator;
        QStringList trPaths = PathInfo::translations();
        bool match = false;
        for (const QString &path: trPaths) {
            match = translator.load(QLocale::system().language(),
                                    "Internationalization", "_",
                                    path);
            if (match) {
                break;
            }
        }

        app.installTranslator(&translator);
        app.setAttribute(Qt::AA_DontCreateNativeWidgetSiblings, true);
        app.setApplicationName("flameshot");
        app.setOrganizationName("Dharkael");

#ifdef Q_OS_LINUX
        auto c = Controller::getInstance();
        new FlameshotDBusAdapter(c);
        QDBusConnection dbus = QDBusConnection::sessionBus();
        if (!dbus.isConnected()) {
            SystemNotification().sendMessage(
                        QObject::tr("Unable to connect via DBus"));
        }
        dbus.registerObject("/", c);
        dbus.registerService("org.dharkael.Flameshot");
#else
        // Create inicial static instance
        Controller::getInstance();
#endif
        return app.exec();
    }

#ifndef Q_OS_WIN
    /*--------------|
     * CLI parsing  |
     * ------------*/
    QCoreApplication app(argc, argv);
    app.setApplicationName("flameshot");
    app.setOrganizationName("Dharkael");
    app.setApplicationVersion(qApp->applicationVersion());
    CommandLineParser parser;
    // Add description
    parser.setDescription(
                "Powerful yet simple to use screenshot software.");
    parser.setGeneralErrorMessage("See 'flameshot --help'.");
    // Arguments
    CommandArgument fullArgument("full", "Capture the entire desktop.");
    CommandArgument guiArgument("gui", "Start a manual capture in GUI mode.");
    CommandArgument configArgument("config", "Configure flameshot.");

    // Options
    CommandOption pathOption(
                {"p", "path"},
                "Path where the capture will be saved",
                "path");
    CommandOption clipboardOption(
                {"c", "clipboard"},
                "Save the capture to the clipboard");
    CommandOption delayOption(
                {"d", "delay"},
                "Delay time in milliseconds",
                "milliseconds");
    CommandOption filenameOption(
                {"f", "filename"},
                "Set the filename pattern",
                "pattern");
    CommandOption trayOption(
                {"t", "trayicon"},
                "Enable or disable the trayicon",
                "bool");
    CommandOption showHelpOption(
                {"s", "showhelp"},
                "Show the help message in the capture mode",
                "bool");
    CommandOption mainColorOption(
                {"m", "maincolor"},
                "Define the main UI color",
                "color-code");
    CommandOption contrastColorOption(
                {"k", "contrastcolor"},
                "Define the contrast UI color",
                "color-code");
    CommandOption rawImageOption(
                {"r", "raw"},
                "Print raw PNG capture");

    // Add checkers
    auto colorChecker = [&parser](const QString &colorCode) -> bool {
        QColor parsedColor(colorCode);
        return parsedColor.isValid() && parsedColor.alphaF() == 1.0;
    };
    QString colorErr = "Invalid color, "
                       "this flag supports the following formats:\n"
                       "- #RGB (each of R, G, and B is a single hex digit)\n"
                       "- #RRGGBB\n- #RRRGGGBBB\n"
                       "- #RRRRGGGGBBBB\n"
                       "- Named colors like 'blue' or 'red'\n"
                       "You may need to escape the '#' sign as in '\\#FFF'";

    const QString delayErr = "Invalid delay, it must be higher than 0";
    auto delayChecker = [&parser](const QString &delayValue) -> bool {
        int value = delayValue.toInt();
        return value >= 0;
    };

    const QString pathErr = "Invalid path, it must be a real path in the system";
    auto pathChecker = [&parser, pathErr](const QString &pathValue) -> bool {
        bool res = QDir(pathValue).exists();
        if (!res) {
            SystemNotification().sendMessage(QObject::tr(pathErr.toLatin1().data()));
        }
        return res;
    };

    const QString booleanErr = "Invalid value, it must be defined as 'true' or 'false'";
    auto booleanChecker = [&parser](const QString &value) -> bool {
        return value == "true" || value == "false";
    };

    contrastColorOption.addChecker(colorChecker, colorErr);
    mainColorOption.addChecker(colorChecker, colorErr);
    delayOption.addChecker(delayChecker, delayErr);
    pathOption.addChecker(pathChecker, pathErr);
    trayOption.addChecker(booleanChecker, booleanErr);
    showHelpOption.addChecker(booleanChecker, booleanErr);

    // Relationships
    parser.AddArgument(guiArgument);
    parser.AddArgument(fullArgument);
    parser.AddArgument(configArgument);
    auto helpOption = parser.addHelpOption();
    auto versionOption = parser.addVersionOption();
    parser.AddOptions({ pathOption, delayOption, rawImageOption }, guiArgument);
    parser.AddOptions({ pathOption, clipboardOption, delayOption, rawImageOption },
                      fullArgument);
    parser.AddOptions({ filenameOption, trayOption, showHelpOption,
                        mainColorOption, contrastColorOption }, configArgument);
    // Parse
    if (!parser.parse(app.arguments())) {
        goto finish;
    }

    // PROCESS DATA
    //--------------
    if (parser.isSet(helpOption) || parser.isSet(versionOption)) {
    }
    else if (parser.isSet(guiArgument)) { // GUI
        QString pathValue = parser.value(pathOption);
        int delay = parser.value(delayOption).toInt();
        bool isRaw = parser.isSet(rawImageOption);
        uint id = qHash(app.arguments().join(" "));
        DBusUtils utils(id);

        // Send message
        QDBusMessage m = QDBusMessage::createMethodCall("org.dharkael.Flameshot",
                                           "/", "", "graphicCapture");
        m << pathValue << delay << id;
        QDBusConnection sessionBus = QDBusConnection::sessionBus();
        utils.checkDBusConnection(sessionBus);
        sessionBus.call(m);

        if (isRaw) {
            // captureTaken
            sessionBus.connect("org.dharkael.Flameshot",
                               "/", "", "captureTaken",
                               &utils,
                               SLOT(captureTaken(uint, QByteArray)));
            // captureFailed
            sessionBus.connect("org.dharkael.Flameshot",
                               "/", "", "captureFailed",
                               &utils,
                               SLOT(captureFailed(uint)));
            QTimer t;
            t.setInterval(1000 * 60 * 15); // 15 minutes timeout
            QObject::connect(&t, &QTimer::timeout, qApp,
                             &QCoreApplication::quit);
            t.start();
            // wait
            app.exec();
        }
    }
    else if (parser.isSet(fullArgument)) { // FULL
        QString pathValue = parser.value(pathOption);
        int delay = parser.value(delayOption).toInt();
        bool toClipboard = parser.isSet(clipboardOption);
        bool isRaw = parser.isSet(rawImageOption);
        // Not a valid command
        if (!isRaw && !toClipboard && pathValue.isEmpty()) {
            QTextStream(stdout) << "you have to set a valid flag:\n\n";
            parser.parse(QStringList() << argv[0] << "full" << "-h");
            goto finish;
        }

        uint id = qHash(app.arguments().join(" "));
        DBusUtils utils(id);

        // Send message
        QDBusMessage m = QDBusMessage::createMethodCall("org.dharkael.Flameshot",
                                               "/", "", "fullScreen");
        m << pathValue << toClipboard << delay << id;
        QDBusConnection sessionBus = QDBusConnection::sessionBus();
        utils.checkDBusConnection(sessionBus);
        sessionBus.call(m);

        if (isRaw) {
            // captureTaken
            sessionBus.connect("org.dharkael.Flameshot",
                               "/", "", "captureTaken",
                               &utils,
                               SLOT(captureTaken(uint, QByteArray)));
            // captureFailed
            sessionBus.connect("org.dharkael.Flameshot",
                               "/", "", "captureFailed",
                               &utils,
                               SLOT(captureFailed(uint)));
            // timeout just in case
            QTimer t;
            t.setInterval(2000);
            QObject::connect(&t, &QTimer::timeout, qApp,
                             &QCoreApplication::quit);
            t.start();
            // wait
            app.exec();
        }
    }
    else if (parser.isSet(configArgument)) { // CONFIG
        bool filename = parser.isSet(filenameOption);
        bool tray = parser.isSet(trayOption);
        bool help = parser.isSet(showHelpOption);
        bool mainColor = parser.isSet(mainColorOption);
        bool contrastColor = parser.isSet(contrastColorOption);
        bool someFlagSet = (filename || tray || help ||
                            mainColor || contrastColor);
        ConfigHandler config;
        if (filename) {
            QString newFilename(parser.value(filenameOption));
            config.setFilenamePattern(newFilename);
            FileNameHandler fh;
            QTextStream(stdout)
                    << QStringLiteral("The new pattern is '%1'\n"
                                      "Parsed pattern example: %2\n").arg(newFilename)
                       .arg(fh.parsedPattern());
        }
        if (tray) {
            QDBusMessage m = QDBusMessage::createMethodCall("org.dharkael.Flameshot",
                                               "/", "", "trayIconEnabled");
            if (parser.value(trayOption) == "false") {
                m << false;
            } else if (parser.value(trayOption) == "true") {
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
            if (parser.value(showHelpOption) == "false") {
                config.setShowHelp(false);
            } else if (parser.value(showHelpOption) == "true") {
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
            QDBusMessage m = QDBusMessage::createMethodCall("org.dharkael.Flameshot",
                                               "/", "", "openConfig");
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
