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
#include "src/utils/confighandler.h"
#include "src/core/controller.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QTextCodec>
#include <QGroupBox>

GeneneralConf::GeneneralConf(QWidget *parent) : QGroupBox(parent) {
    m_layout = new QVBoxLayout(this);
    m_layout->setAlignment(Qt::AlignTop);
    initShowHelp();
    initShowDesktopNotification();
    initShowTrayIcon();
	initConfingButtons();
    updateComponents();
}

void GeneneralConf::updateComponents() {
    ConfigHandler config;
    m_helpMessage->setChecked(config.showHelpValue());
    m_showTray->setChecked(!config.disabledTrayIconValue());
    m_sysNotifications->setChecked(config.desktopNotificationValue());
}

void GeneneralConf::showHelpChanged(bool checked) {
    ConfigHandler().setShowHelp(checked);
}

void GeneneralConf::showDesktopNotificationChanged(bool checked) {
    ConfigHandler().setDesktopNotification(checked);
}

void GeneneralConf::showTrayIconChanged(bool checked) {
    auto controller = Controller::getInstance();
    if (checked) {
        controller->enableTrayIcon();
    } else {
        controller->disableTrayIcon();
	}
}

void GeneneralConf::importConfiguration() {
	QString fileName = QFileDialog::getOpenFileName(this, tr("Import"));
	QFile file(fileName);
	QTextCodec *codec = QTextCodec::codecForLocale();
	if (!file.open(QFile::ReadOnly)) {
		QMessageBox::about(this, tr("Error"), tr("Unable to read file."));
		return;
	}
	QString text = codec->toUnicode(file.readAll());
	file.close();

	QFile config(ConfigHandler().configFilePath());
	if (!config.open(QFile::WriteOnly)) {
	   QMessageBox::about(this, tr("Error"), tr("Unable to write file."));
	   return;
	}
	config.write(codec->fromUnicode(text));
	config.close();
}

void GeneneralConf::exportConfiguration() {
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
							   "flameshot.conf");
	QFile::copy(ConfigHandler().configFilePath(), fileName);
}

void GeneneralConf::resetConfiguration() {
	QMessageBox::StandardButton reply;
	reply = QMessageBox::question(
			  this, tr("Confirm Reset"),
			  tr("Are you sure you want to reset the configuration?"),
			  QMessageBox::Yes | QMessageBox::No);
	if (reply == QMessageBox::Yes) {
		ConfigHandler().setDefaults();
	}
}

void GeneneralConf::initShowHelp() {
    m_helpMessage = new QCheckBox(tr("Show help message"), this);
    ConfigHandler config;
    bool checked = config.showHelpValue();
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
    bool checked = config.desktopNotificationValue();
    m_sysNotifications->setChecked(checked);
    m_sysNotifications->setToolTip(tr("Show desktop notifications"));
    m_layout->addWidget(m_sysNotifications);

    connect(m_sysNotifications, &QCheckBox::clicked, this,
            &GeneneralConf::showDesktopNotificationChanged);
}

void GeneneralConf::initShowTrayIcon() {
    m_showTray = new QCheckBox(tr("Show tray icon"), this);
    ConfigHandler config;
    bool checked = !config.disabledTrayIconValue();
    m_showTray->setChecked(checked);
    m_showTray->setToolTip(tr("Show the systemtray icon"));
    m_layout->addWidget(m_showTray);

    connect(m_showTray, &QCheckBox::clicked, this,
			&GeneneralConf::showTrayIconChanged);
}

void GeneneralConf::initConfingButtons() {

	QHBoxLayout *buttonLayout = new QHBoxLayout();
	m_layout->addStretch();
	QGroupBox *box = new QGroupBox(tr("Configuration File"));
	box->setFlat(true);
	box->setLayout(buttonLayout);
	m_layout->addWidget(box);

	m_exportButton = new QPushButton(tr("Export"));
	buttonLayout->addWidget(m_exportButton);
	connect(m_exportButton, &QPushButton::clicked, this,
			&GeneneralConf::exportConfiguration);

	m_importButton = new QPushButton(tr("Import"));
	buttonLayout->addWidget(m_importButton);
	connect(m_importButton, &QPushButton::clicked, this,
			&GeneneralConf::importConfiguration);

	m_resetButton = new QPushButton(tr("Reset"));
	buttonLayout->addWidget(m_resetButton);
	connect(m_resetButton, &QPushButton::clicked, this,
			&GeneneralConf::resetConfiguration);
}
