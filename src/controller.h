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
#include <QSystemTrayIcon>

class QMenu;
class QSystemTrayIcon;
class QAction;
class CaptureWidget;
class ConfigWindow;
class InfoWindow;

class Controller : public QObject {
    Q_OBJECT
public:
    explicit Controller(QObject *parent = 0);

    QString saveScreenshot(bool toClipboard = false);
    QString saveScreenshot(QString path, bool toClipboard = false);
public slots:
    QPointer<CaptureWidget> createCapture(bool enableSaveWindow = true);
    void createVisualCapture(bool enableSaveWindow = true);
    void openConfigWindow();
    void openInfoWindow();
    void showDesktopNotification(QString);

private slots:
    void initDefaults();
    void trayIconActivated(QSystemTrayIcon::ActivationReason r);

private:
    void createActions();
    void createTrayIcon();

    QAction *m_configAction;
    QAction *m_infoAction;
    QAction *m_quitAction;

    QSystemTrayIcon *m_trayIcon;
    QMenu *m_trayIconMenu;

    QPointer<CaptureWidget> m_captureWindow;
    QPointer<InfoWindow> m_infoWindow;
    QPointer<ConfigWindow> m_configWindow;
};

#endif // CONTROLLER_H
