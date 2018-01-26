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

#include "ftpconfig.h"
#include "src/utils/filenamehandler.h"
#include "src/utils/confighandler.h"
#include "src/config/strftimechooserwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>

FtpConfig::FtpConfig(QWidget *parent) : QWidget(parent) {
    initWidgets();
    initLayout();    
}

void FtpConfig::initLayout() {
    m_layout = new QVBoxLayout(this);
    m_layout->setAlignment(Qt::AlignTop);

    auto infoLabel = new QLabel(tr("FTP connection settings:"), this);
    infoLabel->setFixedHeight(20);
    m_layout->addWidget(infoLabel);

    m_layout->addWidget(new QLabel(tr("Hostname:")));
    m_layout->addWidget(m_hostname);
    m_layout->addWidget(new QLabel(tr("URL to uploaded file (with trailing slash):")));
    m_layout->addWidget(m_sitename);
    m_layout->addWidget(new QLabel(tr("Port:")));
    m_layout->addWidget(m_port);
    m_layout->addWidget(new QLabel(tr("Login:")));
    m_layout->addWidget(m_username);
    m_layout->addWidget(new QLabel(tr("Password:")));
    m_layout->addWidget(m_password);

    QHBoxLayout *horizLayout = new QHBoxLayout();
    horizLayout->addWidget(m_saveButton);
    m_layout->addLayout(horizLayout);
}

void FtpConfig::initWidgets() {

    // hostname
    m_hostname = new QLineEdit(this);

    // sitename
    m_sitename = new QLineEdit(this);

    // port
    m_port = new QLineEdit(this);
    m_port->setInputMask("00000");

    // username
    m_username = new QLineEdit(this);

    // password
    m_password = new QLineEdit(this);
    m_password->setEchoMode(QLineEdit::Password);

    // save
    m_saveButton = new QPushButton(tr("Save"), this);
    connect(m_saveButton, &QPushButton::clicked, this, &FtpConfig::saveSettings);
    m_saveButton->setToolTip(tr("Saves FTP settings"));

    updateComponents();

}

void FtpConfig::saveSettings() {
    ConfigHandler().setFtpHostname(m_hostname->text());
    ConfigHandler().setFtpSite(m_sitename->text());
    ConfigHandler().setFtpPort(m_port->text().toInt());
    ConfigHandler().setFtpLogin(m_username->text());
    ConfigHandler().setFtpPassword(m_password->text());
}

void FtpConfig::updateComponents() {
    m_hostname->setText(ConfigHandler().ftpHostname());
    m_sitename->setText(ConfigHandler().ftpSite());

    int port = ConfigHandler().ftpPort();
    if(port) {
        m_port->setText(QString::number(ConfigHandler().ftpPort()));
    } else {
        m_port->setText("21");
    }

    m_username->setText(ConfigHandler().ftpLogin());
    m_password->setText(ConfigHandler().ftpPassword());
}
