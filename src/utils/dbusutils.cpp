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

#include "dbusutils.h"
#include "src/utils/systemnotification.h"
#include <QApplication>
#include <QTextStream>
#include <QFile>

DBusUtils::DBusUtils(QObject *parent) : QObject(parent) {
}

void DBusUtils::connectPrintCapture(QDBusConnection &session, uint id) {
    m_id = id;
    // captureTaken
    session.connect("org.dharkael.Flameshot",
                       "/", "", "captureTaken",
                       this,
                       SLOT(captureTaken(uint, QByteArray)));
    // captureFailed
    session.connect("org.dharkael.Flameshot",
                       "/", "", "captureFailed",
                       this,
                       SLOT(captureFailed(uint)));
}

void DBusUtils::checkDBusConnection(const QDBusConnection &connection) {
    if (!connection.isConnected()) {
        SystemNotification().sendMessage(tr("Unable to connect via DBus"));
        qApp->exit();
    }
}

void DBusUtils::captureTaken(uint id, QByteArray rawImage) {
    if (m_id == id) {
        QFile file;
        file.open(stdout, QIODevice::WriteOnly);
        file.write(rawImage);
        file.close();
        qApp->exit();
    }
}

void DBusUtils::captureFailed(uint id) {
    if (m_id == id) {
        QTextStream(stdout) << "screenshot aborted\n";
        qApp->exit();
    }
}
