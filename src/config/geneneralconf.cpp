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

#include "geneneralconf.h"
#include "src/utils/confighandler.h"
#include "src/core/controller.h"
#include <QVBoxLayout>
#include <QCheckBox>

GeneneralConf::GeneneralConf(QWidget *parent) : QGroupBox(parent) {
    m_layout = new QVBoxLayout(this);
    m_layout->setAlignment(Qt::AlignTop);
    initShowHelp();
    initShowDesktopNotification();
    initShowTrayIcon();
    updateComponents();
}

void GeneneralConf::updateComponents() {
    ConfigHandler config;
    m_helpMessage->setChecked(config.getShowHelp());
    m_showTray->setChecked(!config.getDisabledTrayIcon());
    m_sysNotifications->setChecked(config.getDesktopNotification());
}

void GeneneralConf::showHelpChanged(bool checked) {
    ConfigHandler().setShowHelp(checked);
}

void GeneneralConf::showDesktopNotificationChanged(bool checked) {
    ConfigHandler().setDesktopNotification(checked);
}

void GeneneralConf::showTrayIconChanged(bool checked) {
    auto controller = Controller::getInstance();
    if(checked) {
        controller->enableTrayIcon();
    } else {
        controller->disableTrayIcon();
    }
}

void GeneneralConf::initShowHelp() {
    m_helpMessage = new QCheckBox(tr("Show help message"), this);
    ConfigHandler config;
    bool checked = config.getShowHelp();
    m_helpMessage->setChecked(checked);
    m_helpMessage->setToolTip(tr("Show the help message at the beginning "
                       "in the capture mode."));
    m_layout->addWidget(m_helpMessage);

    connect(m_helpMessage, &QCheckBox::clicked, this,
            &GeneneralConf::showHelpChanged);
}

void GeneneralConf::initShowDesktopNotification() {
    m_sysNotifications =
            new QCheckBox(tr("Show desktop notifications"), this);
    ConfigHandler config;
    bool checked = config.getDesktopNotification();
    m_sysNotifications->setChecked(checked);
    m_sysNotifications->setToolTip(tr("Show desktop notifications"));
    m_layout->addWidget(m_sysNotifications);

    connect(m_sysNotifications, &QCheckBox::clicked, this,
            &GeneneralConf::showDesktopNotificationChanged);
}

void GeneneralConf::initShowTrayIcon() {
    m_showTray = new QCheckBox(tr("Show tray icon"), this);
    ConfigHandler config;
    bool checked = !config.getDisabledTrayIcon();
    m_showTray->setChecked(checked);
    m_showTray->setToolTip(tr("Show the systemtray icon"));
    m_layout->addWidget(m_showTray);

    connect(m_showTray, &QCheckBox::clicked, this,
            &GeneneralConf::showTrayIconChanged);
}
