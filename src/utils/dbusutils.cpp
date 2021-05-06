// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

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
