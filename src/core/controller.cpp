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

#include "controller.h"
#include "src/capture/widgets/capturewidget.h"
#include "src/utils/confighandler.h"
#include "src/infowindow.h"
#include "src/config/configwindow.h"
#include "src/capture/widgets/capturebutton.h"
#include <QFile>
#include <QApplication>
#include <QSystemTrayIcon>
#include <QAction>
#include <QMenu>

#ifdef Q_OS_WIN
#include "src/core/globalshortcutfilter.h"
#endif

// Controller is the core component of Flameshot, creates the trayIcon and
// launches the capture widget

Controller::Controller() : m_captureWindow(nullptr)
{
    qApp->setQuitOnLastWindowClosed(false);

    // init tray icon
#if defined(Q_OS_LINUX)
    if (!ConfigHandler().disabledTrayIconValue()) {
        enableTrayIcon();
    }
#elif defined(Q_OS_WIN)
    enableTrayIcon();

    GlobalShortcutFilter *nativeFilter = new GlobalShortcutFilter(this);
    qApp->installNativeEventFilter(nativeFilter);
    connect(nativeFilter, &GlobalShortcutFilter::printPressed,
            this, [this](){ this->createVisualCapture(); });
#endif

    QString StyleSheet = CaptureButton::globalStyleSheet();
    qApp->setStyleSheet(StyleSheet);
}

Controller *Controller::getInstance() {
    static Controller c;
    return &c;
}

// creation of a new capture in GUI mode
void Controller::createVisualCapture(const uint id, const QString &forcedSavePath) {
    if (!m_captureWindow) {
        QWidget *modalWidget = nullptr;
        do {
            modalWidget = qApp->activeModalWidget();
            if (modalWidget) {
                modalWidget->close();
                modalWidget->deleteLater();
            }
        } while (modalWidget);

        m_captureWindow = new CaptureWidget(id, forcedSavePath);
        connect(m_captureWindow, &CaptureWidget::captureFailed,
                this, &Controller::captureFailed);
        connect(m_captureWindow, &CaptureWidget::captureTaken,
                this, &Controller::captureTaken);
#ifdef Q_OS_WIN
        m_captureWindow->show();
#else
        m_captureWindow->showFullScreen();
#endif
    }
}

// creation of the configuration window
void Controller::openConfigWindow() {
    if (!m_configWindow) {
        m_configWindow = new ConfigWindow();
        m_configWindow->show();
    }
}

// creation of the window of information
void Controller::openInfoWindow() {
    if (!m_infoWindow) {
        m_infoWindow = new InfoWindow();
    }
}

void Controller::enableTrayIcon() {
    if (m_trayIcon) {
        return;
    }
    ConfigHandler().setDisabledTrayIcon(false);
    QAction *configAction = new QAction(tr("&Configuration"), this);
    connect(configAction, &QAction::triggered, this,
            &Controller::openConfigWindow);
    QAction *infoAction = new QAction(tr("&Information"), this);
    connect(infoAction, &QAction::triggered, this,
            &Controller::openInfoWindow);
    QAction *quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, &QAction::triggered, qApp,
            &QCoreApplication::quit);

    QMenu *trayIconMenu = new QMenu();
    trayIconMenu->addAction(configAction);
    trayIconMenu->addAction(infoAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    m_trayIcon = new QSystemTrayIcon();
    m_trayIcon->setToolTip("Flameshot");
    m_trayIcon->setContextMenu(trayIconMenu);
    m_trayIcon->setIcon(QIcon(":img/flameshot.png"));

    auto trayIconActivated = [this](QSystemTrayIcon::ActivationReason r){
        if (r == QSystemTrayIcon::Trigger) {
            createVisualCapture();
        }
    };
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, trayIconActivated);
    m_trayIcon->show();
}

void Controller::disableTrayIcon() {
#ifdef Q_OS_LINUX
    if (m_trayIcon) {
        m_trayIcon->deleteLater();
    }
    ConfigHandler().setDisabledTrayIcon(true);
#endif
}

void Controller::sendTrayNotification(
        const QString &text,
        const QString &title,
        const int timeout)
{
    if (m_trayIcon) {
        m_trayIcon->showMessage(title, text, QSystemTrayIcon::Information, timeout);
    }
}

void Controller::updateConfigComponents() {
    if (m_configWindow) {
        m_configWindow->updateChildren();
    }
}
