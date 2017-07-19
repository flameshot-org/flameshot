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
#include <QVBoxLayout>
#include <QCheckBox>

GeneneralConf::GeneneralConf(QWidget *parent) : QFrame(parent) {
    setFrameStyle(QFrame::StyledPanel);

    m_layout = new QVBoxLayout(this);
    initShowHelp();
    initShowDesktopNotification();
}

void GeneneralConf::showHelpChanged(bool checked) {
    ConfigHandler().setShowHelp(checked);
}

void GeneneralConf::showDesktopNotificationChanged(bool checked) {
    ConfigHandler().setDesktopNotification(checked);
}

void GeneneralConf::initShowHelp() {
    QCheckBox *c = new QCheckBox(tr("Show help message"), this);
    ConfigHandler config;
    bool checked = config.getShowHelp();
    c->setChecked(checked);
    c->setToolTip(tr("Show the help message at the beginning "
                       "in the capture mode."));
    m_layout->addWidget(c);

    connect(c, &QCheckBox::clicked, this, &GeneneralConf::showHelpChanged);
}

void GeneneralConf::initShowDesktopNotification() {
    QCheckBox *c = new QCheckBox(tr("Show desktop notifications"), this);
    ConfigHandler config;
    bool checked = config.getDesktopNotification();
    c->setChecked(checked);
    c->setToolTip(tr("Show desktop notifications"));
    m_layout->addWidget(c);

    connect(c, &QCheckBox::clicked, this,
            &GeneneralConf::showDesktopNotificationChanged);
}
