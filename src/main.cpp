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
#include <QDBusConnection>

int main(int argc, char *argv[]) {
    QTranslator translator;
    translator.load(QLocale::system().language(),
      "Internationalization", "_", "/usr/share/flameshot/translations/");

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
