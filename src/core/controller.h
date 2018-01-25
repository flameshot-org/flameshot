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

#pragma once

#include <QObject>
#include <QPointer>
#include <QPixmap>

class CaptureWidget;
class ConfigWindow;
class InfoWindow;
class QSystemTrayIcon;

class Controller : public QObject {
    Q_OBJECT

public:
    static Controller* getInstance();

    Controller(const Controller&) = delete;
    void operator =(const Controller&) = delete;

signals:
    void captureTaken(uint id, QByteArray p);
    void captureFailed(uint id);

public slots:
    void createVisualCapture(const uint id = 0,
                             const QString &forcedSavePath = QString());

    void openConfigWindow();
    void openInfoWindow();

    void enableTrayIcon();
    void disableTrayIcon();
    void sendTrayNotification(const QString &text,
                              const QString &title = "Flameshot Info",
                              const int timeout = 5000);

    void updateConfigComponents();

private slots:

private:
    Controller();

    QPointer<CaptureWidget> m_captureWindow;
    QPointer<InfoWindow> m_infoWindow;
    QPointer<ConfigWindow> m_configWindow;
    QPointer<QSystemTrayIcon> m_trayIcon;

};
