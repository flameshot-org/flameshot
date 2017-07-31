// Copyright 2017 Alejandro Sirgo Rica
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
#include "src/core/flameshotdbusadapter.h"
#include "src/utils/filenamehandler.h"
#include "src/utils/confighandler.h"
#include <QApplication>
#include <QTranslator>
#include <QObject>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QCommandLineParser>
#include <QDir>

int main(int argc, char *argv[]) {
    // required for the button serialization
    qRegisterMetaTypeStreamOperators<QList<int> >("QList<int>");
    qApp->setApplicationVersion(static_cast<QString>(APP_VERSION));

    QTranslator translator;
    translator.load(QLocale::system().language(),
      "Internationalization", "_", "/usr/share/flameshot/translations/");

    // no arguments, just launch Flameshot
    if (argc == 1) {
        SingleApplication app(argc, argv);
        app.installTranslator(&translator);
        app.setAttribute(Qt::AA_DontCreateNativeWidgetSiblings, true);
        app.setApplicationName("flameshot");
        app.setOrganizationName("Dharkael");

        auto c = Controller::getInstance();
        new FlameshotDBusAdapter(c);
        QDBusConnection dbus = QDBusConnection::sessionBus();
        dbus.registerObject("/", c);
        dbus.registerService("org.dharkael.Flameshot");
        return app.exec();
    }

    /*--------------|
     * CLI parsing  |
     * ------------*/
    QCoreApplication app(argc, argv);
    app.setApplicationName("flameshot");
    app.setOrganizationName("Dharkael");
    app.setApplicationVersion(qApp->applicationVersion());
    QCommandLineParser parser;
    // Add description
    parser.setApplicationDescription(
                "Powerfull yet simple to use screenshot software.");
    // Command descriptions
    QString fullDescription = "Capture the entire desktop.";
    QString guiDescription = "Start a manual capture in GUI mode.";
    QString configDescription = "Configure flameshot.";
    // Positional alguments
    parser.addPositionalArgument("mode", "full\t" + fullDescription + "\n"+
                                 "gui\t" + guiDescription + "\n" +
                                 "config\t" + configDescription,
                                 "mode [mode_options]");

    // Add options
    parser.addHelpOption();
    parser.addVersionOption();
    // Initial parse ---------------------------------
    parser.parse(app.arguments());
    QTextStream out(stdout);

    // CLI options
    QCommandLineOption pathOption(QStringList() << "p" << "path",
                                  "Path where the capture will be saved", "path");
    QCommandLineOption clipboardOption({{"c", "clipboard"},
                                        "Save the capture to the clipboard"});
    QCommandLineOption delayOption(QStringList() << "d" << "delay",
                                  "Delay time in milliseconds", "milliseconds");
    QCommandLineOption filenameOption("filename", "Set the filename pattern", "pattern");
    QCommandLineOption trayOption("trayicon", "Enable or disable the trayicon", "bool");
    QCommandLineOption showHelpOption("showhelp", "Show the help message in the capture mode", "bool");
    // add here the names of the options without required values after the tag
    QStringList optionsWithoutValue = QStringList()
            << clipboardOption.names();
    // Second parse ----------------------------------

    /* Detect undesired elements
     * This is a very hacky solution to filter undesired arguments.
     * I may consider changing to a better cli parsing library for
     * this kind of command structure.
     */
    auto args = app.arguments().mid(1); // ignore the first
    QStringList commandList{"gui", "full", "config"};
    auto i = args.cbegin();
    QString val = (*i);
    bool ok = commandList.contains(val);
    // check first
    for (++i; i != args.cend(); ++i) {
        if (!ok) break;
        val = (*i);
        if(val.startsWith("-")) {
            // skip next when
            // - the parameter is not in the format -flag=100
            // - there are more elements to check
            // - it's a flag and it requires a value
            bool skipNext = (!val.contains("=") && i+1 != args.cend()
                    && !optionsWithoutValue.contains(val.remove("-")));
            if (skipNext) ++i;
        } else { // not a flag
            ok = false;
        }
    }

    // obtain the command
    QString command;
    if (ok && parser.positionalArguments().count() > 0) {
        command = parser.positionalArguments().first();
    }

    // GUI
    if (command == "gui") {
        // Description
        parser.clearPositionalArguments();
        parser.addPositionalArgument(
                    "gui", guiDescription, "gui [gui_options]");
        parser.addOptions({ pathOption, delayOption });
        parser.process(app);

        // paramenters
        QString pathValue;
        if (parser.isSet(pathOption)) {
            pathValue = QString::fromStdString(parser.value("path").toStdString());
            if (!QDir(pathValue).exists()) {
                qWarning().noquote() << "Invalid path.";
                return 0;
            }
        }
        int delay = 0;
        if (parser.isSet(delayOption)) {
            delay = parser.value("delay").toInt();
            if (delay < 0) {
                qWarning().noquote() << "Invalid negative delay.";
                return 0;
            }
        }

        // Send message
        QDBusMessage m = QDBusMessage::createMethodCall("org.dharkael.Flameshot",
                                           "/", "", "graphicCapture");
        m << pathValue << delay;
        QDBusConnection::sessionBus().call(m);

    }
    // FULL
    else if (command == "full") {
        // Description
        parser.clearPositionalArguments();
        parser.addPositionalArgument(
                    "full", fullDescription, "full [full_options]");
        parser.addOptions({ pathOption, clipboardOption, delayOption });
        parser.process(app);

        // paramenters
        QString pathValue;
        if (parser.isSet(pathOption)) {
            pathValue = QString::fromStdString(parser.value("path").toStdString());
            if (!QDir(pathValue).exists()) {
                qWarning().noquote() << "Invalid path.";
                return 0;
            }
        }
        int delay = 0;
        if (parser.isSet(delayOption)) {
            delay = parser.value("delay").toInt();
            if (delay < 0) {
                qWarning().noquote() << "Invalid negative delay.";
                return 0;
            }
        }

        // Send message
        QDBusMessage m = QDBusMessage::createMethodCall("org.dharkael.Flameshot",
                                           "/", "", "fullScreen");
        m << pathValue << parser.isSet("clipboard") << delay;
        QDBusConnection::sessionBus().call(m);

    }
    // CONFIG
    else if (command == "config") {
        // Description
        parser.clearPositionalArguments();
        parser.addPositionalArgument(
                    "config", configDescription, "config [config_options]");
        parser.addOptions({ filenameOption, trayOption, showHelpOption });
        parser.process(app);

        bool filename = parser.isSet(filenameOption);
        bool tray = parser.isSet(trayOption);
        bool help = parser.isSet(showHelpOption);
        bool someFlagSet = (filename || tray || help);
        ConfigHandler config;
        if (filename) {
            QString newFilename(parser.value(filenameOption));
            config.setFilenamePattern(newFilename);
            FileNameHandler fh;
            out << "The new pattern is " << newFilename
                              << "\nParsed pattern example: "
                              << fh.getParsedPattern() << "\n";
        }
        if (tray) {
            QDBusMessage m = QDBusMessage::createMethodCall("org.dharkael.Flameshot",
                                               "/", "", "trayIconEnabled");
            if (parser.value(trayOption) == "false") {
                m << false;
            } else if (parser.value(trayOption) == "true") {
                m << true;
            }
            QDBusConnection::sessionBus().call(m);
        }
        if (help) {
            if (parser.value(showHelpOption) == "false") {
                config.setShowHelp(false);
            } else if (parser.value(showHelpOption) == "true") {
                config.setShowHelp(true);
            }
        }
        // Open gui when no options
        if (!someFlagSet) {
            QDBusMessage m = QDBusMessage::createMethodCall("org.dharkael.Flameshot",
                                               "/", "", "openConfig");
            QDBusConnection::sessionBus().call(m);
        }
    } else {
        qWarning().noquote() << "Invalid command, see 'flameshot --help'.";
        parser.process(app.arguments());
    }
    return 0;
}
