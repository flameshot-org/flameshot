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

#include "controller.h"
#include "capture/capturewidget.h"
#include "infowindow.h"
#include "config/configwindow.h"
#include "capture/button.h"
#include <QAction>
#include <QApplication>
#include <QMenu>
#include <QSystemTrayIcon>
#include <QSettings>
#include <QFile>

// Controller is the core component of Flameshot, creates the trayIcon and
// launches the capture widget

Controller::Controller(QObject *parent) : QObject(parent),
        m_captureWindow(nullptr) {
    // required for the button serialization
    qRegisterMetaTypeStreamOperators<QList<int> >("QList<int>");
    createActions();
    createTrayIcon();
    m_trayIcon->show();

    initDefaults();
    qApp->setQuitOnLastWindowClosed(false);

    m_nativeEventFilter = new NativeEventFilter(this);
    qApp->installNativeEventFilter(m_nativeEventFilter);
    connect(m_nativeEventFilter, &NativeEventFilter::activated, this, &Controller::slotPrintHotkey);


    QString StyleSheet = Button::getStyle();
    qApp->setStyleSheet(StyleSheet);

}

// creates the items of the trayIcon
void Controller::createActions() {
    m_configAction = new QAction(tr("&Configuration"), this);
    connect(m_configAction, &QAction::triggered, this, &Controller::openConfigWindow);

    m_infoAction = new QAction(tr("&Information"), this);
    connect(m_infoAction, &QAction::triggered, this, &Controller::openInfoWindow);

    m_quitAction = new QAction(tr("&Quit"), this);
    connect(m_quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
}

// creates the trayIcon
void Controller::createTrayIcon() {
    // requires a widget as parent but it should be used the whole app live period
    m_trayIconMenu = new QMenu();
    m_trayIconMenu->addAction(m_configAction);
    m_trayIconMenu->addAction(m_infoAction);
    m_trayIconMenu->addSeparator();
    m_trayIconMenu->addAction(m_quitAction);

    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setToolTip("Flameshot");
    m_trayIcon->setContextMenu(m_trayIconMenu);
    m_trayIcon->setIcon(QIcon(":img/flameshot.svg"));
}

// initDefaults inits the global config in the very first run of the program
void Controller::initDefaults() {
    QSettings settings;
    //settings.setValue("initiated", false); // testing change
    if (!settings.value("initiated").toBool()) {
        settings.setValue("initiated", true);
        settings.setValue("drawColor", QColor(Qt::red));
        //settings.setValue("mouseVisible", false);
        settings.setValue("whiteIconColor", true);
        settings.setValue("uiColor", QColor(136, 0, 170));

        QList<int> buttons;
        for (int i = 0; i < static_cast<int>(Button::Type::last); ++i) {
            buttons << i;
        }
        settings.setValue("buttons", QVariant::fromValue(buttons));
    }
}

// creation of a new capture
void Controller::slotPrintHotkey() {
    if (!m_captureWindow) {
        m_captureWindow = new CaptureWidget();
        connect(m_captureWindow, &CaptureWidget::newMessage,
                this, &Controller::showMessage);
    }
}

// creation of the configuration window
void Controller::openConfigWindow() {
    if (!m_configWindow) {
        m_configWindow = new ConfigWindow();
    }
}

// creation of the window of information
void Controller::openInfoWindow() {
    if (!m_infoWindow) {
        m_infoWindow = new InfoWindow();
    }
}

void Controller::showMessage(QString msg) {
    m_trayIcon->showMessage("Flameshot Info", msg);
}
