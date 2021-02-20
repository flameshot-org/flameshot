// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

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

private:
    QDBusInterface* m_interface;
};
