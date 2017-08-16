// Copyright 2017 Alejandro Sirgo Rica
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

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <QPointer>

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

public slots:
    void createVisualCapture(const QString &forcedSavePath = QString());

    void openConfigWindow();
    void openInfoWindow();

    void enableTrayIcon();
    void disableTrayIcon();

    void updateConfigComponents();

private slots:
    void initDefaults();

private:
    Controller();

    QPointer<CaptureWidget> m_captureWindow;
    QPointer<InfoWindow> m_infoWindow;
    QPointer<ConfigWindow> m_configWindow;
    QPointer<QSystemTrayIcon> m_trayIcon;

};

#endif // CONTROLLER_H
