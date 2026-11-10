// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QHash>
#include <QObject>
#include <QString>

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

#if !(defined(Q_OS_MACOS) || defined(Q_OS_WIN))
private slots:
    void onActionInvoked(uint id, const QString& actionKey);
    void onNotificationClosed(uint id, uint reason);

private:
    void ensureInterface();
    void queryCapabilities();

    QDBusInterface* m_interface;
    QHash<uint, QString> m_pendingFolders;
    bool m_supportsActions = true;
    bool m_capabilitiesRequested = false;
    bool m_signalsConnected = false;
#endif
};
