// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QMap>
#include <QObject>

class QDBusInterface;

class SystemNotification : public QObject
{
    Q_OBJECT
public:
    explicit SystemNotification(QObject* parent = nullptr);

    void sendMessage(const QString& text, const QString& savePath = {});

    void sendMessage(const QString& text,
                     const QString& title,
                     const QString& savePath,
                     const int timeout = 5000);

    static bool hasPendingPaths();
    static void setExitOnLastAction(bool exit);
    static void registerNotificationPath(uint id, const QString& path);
    static void registerNotificationPath(const QMap<uint, QString>& paths);
    static QMap<uint, QString> takePendingPaths();

    static void addPendingDaemonNotification(const QString& path);
    static QStringList takePendingDaemonNotifications();

private:
    Q_SLOT void onActionInvoked(uint id, const QString& actionKey);

    static SystemNotification* actionHandler();

    QDBusInterface* m_interface;
    static QMap<uint, QString> s_pendingPaths;
    static QStringList s_pendingDaemonNotifications;
    static bool s_exitOnLastAction;
};
