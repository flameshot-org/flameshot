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
