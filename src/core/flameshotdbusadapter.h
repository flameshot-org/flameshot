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

#include "src/core/controller.h"
#include <QtDBus/QDBusAbstractAdaptor>

class FlameshotDBusAdapter : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.flameshot.Flameshot")

public:
    explicit FlameshotDBusAdapter(QObject* parent = nullptr);
    virtual ~FlameshotDBusAdapter();

signals:
    void captureTaken(uint id, QByteArray rawImage, QRect selection);
    void captureFailed(uint id);

public slots:
    Q_NOREPLY void graphicCapture(QString path, int delay, uint id);
    Q_NOREPLY void fullScreen(QString path,
                              bool toClipboard,
                              int delay,
                              uint id);
    Q_NOREPLY void captureScreen(int number,
                                 QString path,
                                 bool toClipboard,
                                 int delay,
                                 uint id);
    Q_NOREPLY void openLauncher();
    Q_NOREPLY void openConfig();
    Q_NOREPLY void trayIconEnabled(bool enabled);
    Q_NOREPLY void autostartEnabled(bool enabled);

private slots:
    void handleCaptureTaken(uint id, const QPixmap& p, QRect selection);
};
