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

#include "generalconf.h"
#include "src/core/controller.h"
#include "src/utils/confighandler.h"
#include <QCheckBox>
#include <QFile>
#include <QFileDialog>
#include <QGroupBox>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardPaths>
#include <QTextCodec>
#include <QVBoxLayout>

GeneralConf::GeneralConf(QWidget* parent)
  : QWidget(parent)
{
    m_layout = new QVBoxLayout(this);
    m_layout->setAlignment(Qt::AlignTop);
    initShowHelp();
    initShowSidePanelButton();
    initShowDesktopNotification();
    initShowTrayIcon();
    initAutostart();
    initUseJpgForClipboard();
    initSaveAfterCopy();

    // this has to be at the end
    initConfigButtons();
    updateComponents();
}

void GeneralConf::updateComponents()
{
    ConfigHandler config;
    m_helpMessage->setChecked(config.showHelpValue());
    m_sidePanelButton->setChecked(config.showSidePanelButtonValue());
    m_sysNotifications->setChecked(config.desktopNotificationValue());
    m_autostart->setChecked(config.startupLaunchValue());
    m_saveAfterCopy->setChecked(config.saveAfterCopyValue());
    m_useJpgForClipboard->setChecked(config.useJpgForClipboard());

    if (!config.savePath().isEmpty()) {
        m_savePath->setText(config.savePath());
    } else {
        ConfigHandler().setSavePath(
          QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
    }
#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
    m_showTray->setChecked(!config.disabledTrayIconValue());
#endif
}

void GeneralConf::showHelpChanged(bool checked)
{
    ConfigHandler().setShowHelp(checked);
}

void GeneralConf::showSidePanelButtonChanged(bool checked)
{
    ConfigHandler().setShowSidePanelButton(checked);
}

void GeneralConf::showDesktopNotificationChanged(bool checked)
{
    ConfigHandler().setDesktopNotification(checked);
}

void GeneralConf::showTrayIconChanged(bool checked)
{
    auto controller = Controller::getInstance();
    if (checked) {
        controller->enableTrayIcon();
    } else {
        controller->disableTrayIcon();
    }
}

void GeneralConf::autostartChanged(bool checked)
{
    ConfigHandler().setStartupLaunch(checked);
}

void GeneralConf::importConfiguration()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Import"));
    if (fileName.isEmpty()) {
        return;
    }
    QFile file(fileName);
    QTextCodec* codec = QTextCodec::codecForLocale();
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

void GeneralConf::exportFileConfiguration()
{
    QString fileName = QFileDialog::getSaveFileName(
      this, tr("Save File"), QStringLiteral("flameshot.conf"));

    // Cancel button
    if (fileName.isNull()) {
        return;
    }

    QFile targetFile(fileName);
    if (targetFile.exists()) {
        targetFile.remove();
    }
    bool ok = QFile::copy(ConfigHandler().configFilePath(), fileName);
    if (!ok) {
        QMessageBox::about(this, tr("Error"), tr("Unable to write file."));
    }
}

void GeneralConf::resetConfiguration()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(
      this,
      tr("Confirm Reset"),
      tr("Are you sure you want to reset the configuration?"),
      QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        m_savePath->setText(
          QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
        ConfigHandler().setDefaults();
    }
}

void GeneralConf::initShowHelp()
{
    m_helpMessage = new QCheckBox(tr("Show help message"), this);
    ConfigHandler config;
    bool checked = config.showHelpValue();
    m_helpMessage->setChecked(checked);
    m_helpMessage->setToolTip(tr("Show the help message at the beginning "
                                 "in the capture mode."));
    m_layout->addWidget(m_helpMessage);

    connect(
      m_helpMessage, &QCheckBox::clicked, this, &GeneralConf::showHelpChanged);
}

void GeneralConf::initShowSidePanelButton()
{
    m_sidePanelButton = new QCheckBox(tr("Show the side panel button"), this);
    m_sidePanelButton->setChecked(ConfigHandler().showSidePanelButtonValue());
    m_sidePanelButton->setToolTip(
      tr("Show the side panel toggle button in the capture mode."));
    m_layout->addWidget(m_sidePanelButton);

    connect(m_sidePanelButton,
            &QCheckBox::clicked,
            this,
            &GeneralConf::showSidePanelButtonChanged);
}
void GeneralConf::initShowDesktopNotification()
{
    m_sysNotifications = new QCheckBox(tr("Show desktop notifications"), this);
    ConfigHandler config;
    bool checked = config.desktopNotificationValue();
    m_sysNotifications->setChecked(checked);
    m_sysNotifications->setToolTip(tr("Show desktop notifications"));
    m_layout->addWidget(m_sysNotifications);

    connect(m_sysNotifications,
            &QCheckBox::clicked,
            this,
            &GeneralConf::showDesktopNotificationChanged);
}

void GeneralConf::initShowTrayIcon()
{
#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
    m_showTray = new QCheckBox(tr("Show tray icon"), this);
    ConfigHandler config;
    bool checked = !config.disabledTrayIconValue();
    m_showTray->setChecked(checked);
    m_showTray->setToolTip(tr("Show the systemtray icon"));
    m_layout->addWidget(m_showTray);

    connect(m_showTray,
            &QCheckBox::stateChanged,
            this,
            &GeneralConf::showTrayIconChanged);
#endif
}

void GeneralConf::initConfigButtons()
{
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_layout->addStretch();
    QGroupBox* box = new QGroupBox(tr("Configuration File"));
    box->setFlat(true);
    box->setLayout(buttonLayout);
    m_layout->addWidget(box);

    m_exportButton = new QPushButton(tr("Export"));
    buttonLayout->addWidget(m_exportButton);
    connect(m_exportButton,
            &QPushButton::clicked,
            this,
            &GeneralConf::exportFileConfiguration);

    m_importButton = new QPushButton(tr("Import"));
    buttonLayout->addWidget(m_importButton);
    connect(m_importButton,
            &QPushButton::clicked,
            this,
            &GeneralConf::importConfiguration);

    m_resetButton = new QPushButton(tr("Reset"));
    buttonLayout->addWidget(m_resetButton);
    connect(m_resetButton,
            &QPushButton::clicked,
            this,
            &GeneralConf::resetConfiguration);
}

void GeneralConf::initAutostart()
{
    m_autostart = new QCheckBox(tr("Launch at startup"), this);
    ConfigHandler config;
    bool checked = config.startupLaunchValue();
    m_autostart->setChecked(checked);
    m_autostart->setToolTip(tr("Launch Flameshot"));
    m_layout->addWidget(m_autostart);

    connect(
      m_autostart, &QCheckBox::clicked, this, &GeneralConf::autostartChanged);
}

void GeneralConf::initSaveAfterCopy()
{
    m_saveAfterCopy = new QCheckBox(tr("Save image after copy"), this);
    m_saveAfterCopy->setToolTip(tr("Save image file after copying it"));
    m_layout->addWidget(m_saveAfterCopy);
    connect(m_saveAfterCopy,
            &QCheckBox::clicked,
            this,
            &GeneralConf::saveAfterCopyChanged);

    QGroupBox* box = new QGroupBox(tr("Save Path"));
    box->setFlat(true);
    m_layout->addWidget(box);
    m_layout->addStretch();

    QVBoxLayout* vboxLayout = new QVBoxLayout();
    box->setLayout(vboxLayout);

    QHBoxLayout* pathLayout = new QHBoxLayout();

    QString path = ConfigHandler().savePath();
    if (path.isEmpty()) {
        path =
          QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    }
    m_savePath = new QLineEdit(path, this);
    m_savePath->setDisabled(true);
    QString foreground = this->palette().foreground().color().name();
    m_savePath->setStyleSheet(QStringLiteral("color: %1").arg(foreground));
    pathLayout->addWidget(m_savePath);

    m_changeSaveButton = new QPushButton(tr("Change..."), this);
    pathLayout->addWidget(m_changeSaveButton);
    connect(m_changeSaveButton,
            &QPushButton::clicked,
            this,
            &GeneralConf::changeSavePath);

    m_screenshotPathFixedCheck =
      new QCheckBox(tr("Use fixed path for screenshots to save"), this);
    m_screenshotPathFixedCheck->setChecked(ConfigHandler().savePathFixed());
    connect(m_screenshotPathFixedCheck,
            SIGNAL(toggled(bool)),
            this,
            SLOT(togglePathFixed()));

    vboxLayout->addLayout(pathLayout);
    vboxLayout->addWidget(m_screenshotPathFixedCheck);
}

void GeneralConf::initUseJpgForClipboard()
{
    m_useJpgForClipboard =
      new QCheckBox(tr("Use JPG format for clipboard (PNG default)"), this);
    ConfigHandler config;
    bool checked = config.useJpgForClipboard();
    m_useJpgForClipboard->setChecked(checked);
    m_useJpgForClipboard->setToolTip(
      tr("Use JPG format for clipboard (PNG default)"));
    m_layout->addWidget(m_useJpgForClipboard);

    connect(m_useJpgForClipboard,
            &QCheckBox::clicked,
            this,
            &GeneralConf::useJpgForClipboardChanged);
}

void GeneralConf::saveAfterCopyChanged(bool checked)
{
    ConfigHandler().setSaveAfterCopy(checked);
}

void GeneralConf::changeSavePath()
{
    QString path = ConfigHandler().savePath();
    if (path.isEmpty()) {
        path =
          QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    }
    path = chooseFolder(path);
    if (!path.isEmpty()) {
        m_savePath->setText(path);
        ConfigHandler().setSavePath(path);
    }
}

const QString GeneralConf::chooseFolder(const QString pathDefault)
{
    QString path;
    if (pathDefault.isEmpty()) {
        path =
          QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    }
    path = QFileDialog::getExistingDirectory(
      this,
      tr("Choose a Folder"),
      path,
      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (path.isEmpty()) {
        return path;
    }
    if (!path.isEmpty()) {
        if (!QFileInfo(path).isWritable()) {
            QMessageBox::about(
              this, tr("Error"), tr("Unable to write to directory."));
            return QString();
        }
    }
    return path;
}

void GeneralConf::togglePathFixed()
{
    ConfigHandler().setSavePathFixed(m_screenshotPathFixedCheck->isChecked());
}

void GeneralConf::useJpgForClipboardChanged(bool checked)
{
    ConfigHandler().setUseJpgForClipboard(checked);
}
