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

#include "controller.h"
#include "singleapplication.h"
#include "src/flameshotdbusadapter.h"
#include <QApplication>
#include <QTranslator>
#include <QObject>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QCommandLineParser>
#include <QDir>
#include <QDebug>

int main(int argc, char *argv[]) {
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

        Controller c;
        new FlameshotDBusAdapter(&c);
        QDBusConnection dbus = QDBusConnection::sessionBus();
        dbus.registerObject("/", &c);
        dbus.registerService("org.dharkael.Flameshot");
        return app.exec();
    }

    QCoreApplication app(argc, argv);
    app.installTranslator(&translator);
    app.setApplicationName("flameshot");
    app.setOrganizationName("Dharkael");

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.setApplicationDescription(
                QObject::tr("Powerfull yet simple to use screenshot software."));

    QString fullDescription = QObject::tr("Capture the entire desktop.");
    QString guiDescription = QObject::tr("Start a manual capture in GUI mode.");
    parser.addPositionalArgument("mode", "full\t"+fullDescription+"\n"
                                         "gui\t"+guiDescription,
                                 "mode [mode_options]");
    parser.parse(app.arguments());

    const QStringList args = parser.positionalArguments();
    const QString command = args.isEmpty() ? QString() : args.first();

    QCommandLineOption pathOption(QStringList() << "p" << "path",
                                  QObject::tr("Path where the capture will be saved"),
                                  "pathVal");
    QCommandLineOption clipboardOption({{"c", "clipboard"},
                                        QObject::tr("Save the capture to the clipboard")});
    if (command == "full") {
        parser.clearPositionalArguments();
        parser.addPositionalArgument(
                    "full", fullDescription, "full [full_options]");
        parser.addOptions({ pathOption, clipboardOption });
    } else if (command == "gui") {
        parser.clearPositionalArguments();
        parser.addPositionalArgument(
                    "gui", guiDescription, "gui [gui_options]");

        parser.addOption(pathOption);
    }
    parser.process(app);

    QDBusMessage m;
    QString pathValue;
    bool pathIsSetAndValid = parser.isSet("path");
    if (pathIsSetAndValid) {
        pathValue = QString::fromStdString(parser.value("path").toStdString());
        pathIsSetAndValid = QDir(pathValue).exists();
        if (!pathIsSetAndValid) {
            qWarning() << QObject::tr("Invalid path.");
            return 0;
        }
    }

    if (command == "gui") {
        if (pathIsSetAndValid) {
            m = QDBusMessage::createMethodCall("org.dharkael.Flameshot",
                                               "/", "", "openCaptureWithPath");
            m << pathValue;
        } else {
            m = QDBusMessage::createMethodCall("org.dharkael.Flameshot",
                                               "/", "", "openCapture");
        }
        QDBusConnection::sessionBus().call(m);

    } else if (command == "full") {
        if (pathIsSetAndValid) {
            m = QDBusMessage::createMethodCall("org.dharkael.Flameshot",
                                               "/", "", "fullScreenWithPath");
            m << pathValue;
        } else {
            m = QDBusMessage::createMethodCall("org.dharkael.Flameshot",
                                               "/", "", "fullScreen");
        }
        m << parser.isSet("clipboard");
        QDBusConnection::sessionBus().call(m);
    }
    return 0;
}
