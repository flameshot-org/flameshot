// Copyright(c) 2017-2019 Alejandro Sirgo Rica & Contributors
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
#include <QFile>
#include <QRect>
#include <QTextStream>

DBusUtils::DBusUtils(QObject* parent)
  : QObject(parent)
{}

void DBusUtils::connectPrintCapture(QDBusConnection& session, uint id)
{
    m_id = id;
    // captureTaken
    session.connect(QStringLiteral("org.flameshot.Flameshot"),
                    QStringLiteral("/"),
                    QLatin1String(""),
                    QStringLiteral("captureTaken"),
                    this,
                    SLOT(captureTaken(uint, QByteArray, QRect)));
    // captureFailed
    session.connect(QStringLiteral("org.flameshot.Flameshot"),
                    QStringLiteral("/"),
                    QLatin1String(""),
                    QStringLiteral("captureFailed"),
                    this,
                    SLOT(captureFailed(uint)));
}

void DBusUtils::connectSelectionCapture(QDBusConnection& session, uint id)
{
    m_id = id;
    // captureTaken
    session.connect(QStringLiteral("org.flameshot.Flameshot"),
                    QStringLiteral("/"),
                    QLatin1String(""),
                    QStringLiteral("captureTaken"),
                    this,
                    SLOT(selectionTaken(uint, QByteArray, QRect)));
    // captureFailed
    session.connect(QStringLiteral("org.flameshot.Flameshot"),
                    QStringLiteral("/"),
                    QLatin1String(""),
                    QStringLiteral("captureFailed"),
                    this,
                    SLOT(captureFailed(uint)));
}

void DBusUtils::checkDBusConnection(const QDBusConnection& connection)
{
    if (!connection.isConnected()) {
        SystemNotification().sendMessage(tr("Unable to connect via DBus"));
        qApp->exit(1);
    }
}

void DBusUtils::captureTaken(uint id, QByteArray rawImage, QRect selection)
{
    if (m_id == id) {
        QFile file;
        file.open(stdout, QIODevice::WriteOnly);

        file.write(rawImage);
        file.close();
        qApp->exit();
    }
}

void DBusUtils::captureFailed(uint id)
{
    if (m_id == id) {
        QTextStream(stdout) << "screenshot aborted\n";
        qApp->exit(1);
    }
}

void DBusUtils::selectionTaken(uint id, QByteArray rawImage, QRect selection)
{
    if (m_id == id) {
        QFile file;
        file.open(stdout, QIODevice::WriteOnly);

        QTextStream out(&file);
        out << selection.width() << " " << selection.height() << " "
            << selection.x() << " " << selection.y();
        file.close();
        qApp->exit();
    }
}
