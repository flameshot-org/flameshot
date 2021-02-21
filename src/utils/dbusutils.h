// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include "src/cli/commandlineparser.h"
#include <QDBusConnection>
#include <QObject>

class DBusUtils : public QObject
{
    Q_OBJECT
public:
    explicit DBusUtils(QObject* parent = nullptr);

    void connectPrintCapture(QDBusConnection& session, uint id);
    void checkDBusConnection(const QDBusConnection& connection);
    void connectSelectionCapture(QDBusConnection& session, uint id);

public slots:
    void selectionTaken(uint id, QByteArray rawImage, QRect selection);
    void captureTaken(uint id, QByteArray rawImage, QRect selection);
    void captureFailed(uint id);

private:
    uint m_id;
};
