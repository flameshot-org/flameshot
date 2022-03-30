// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors
#include "generalconf.h"
#include "src/core/flameshot.h"
#include "src/utils/confighandler.h"
#include <QCheckBox>
#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QGroupBox>
#include <QImageWriter>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QSizePolicy>
#include <QSpinBox>
#include <QStandardPaths>
#include <QTextCodec>
#include <QVBoxLayout>

GeneralConf::GeneralConf(QWidget* parent)
  : QWidget(parent)
  , m_historyConfirmationToDelete(nullptr)
  , m_undoLimit(nullptr)
{
    m_layout = new QVBoxLayout(this);
    m_layout->setAlignment(Qt::AlignTop);

    // Scroll area adapts the size of the content on small screens.
    // It must be initialized before the checkboxes.
    initScrollArea();

    initShowHelp();
    initShowSidePanelButton();
    initShowDesktopNotification();
    initShowTrayIcon();
    initHistoryConfirmationToDelete();
    initCheckForUpdates();
    initAutostart();
    initShowStartupLaunchMessage();
    initCopyAndCloseAfterUpload();
    initCopyPathAfterSave();
    initAntialiasingPinZoom();
    initUploadWithoutConfirmation();
    initUseJpgForClipboard();
    initSaveAfterCopy();
    initUploadHistoryMax();
    initUndoLimit();
    initUploadClientSecret();
    initAllowMultipleGuiInstances();
#if !defined(Q_OS_WIN)
    initAutoCloseIdleDaemon();
#endif
    initPredefinedColorPaletteLarge();
    initCopyOnDoubleClick();

    m_layout->addStretch();

    initShowMagnifier();
    initSquareMagnifier();
    // this has to be at the end
    initConfigButtons();
    updateComponents();
}

void GeneralConf::_updateComponents(bool allowEmptySavePath)
{
    ConfigHandler config;
    m_helpMessage->setChecked(config.showHelp());
    m_sidePanelButton->setChecked(config.showSidePanelButton());
    m_sysNotifications->setChecked(config.showDesktopNotification());
    m_autostart->setChecked(config.startupLaunch());
    m_copyAndCloseAfterUpload->setChecked(config.copyAndCloseAfterUpload());
    m_saveAfterCopy->setChecked(config.saveAfterCopy());
    m_copyPathAfterSave->setChecked(config.copyPathAfterSave());
    m_antialiasingPinZoom->setChecked(config.antialiasingPinZoom());
    m_useJpgForClipboard->setChecked(config.useJpgForClipboard());
    m_uploadWithoutConfirmation->setChecked(config.uploadWithoutConfirmation());
    m_historyConfirmationToDelete->setChecked(
      config.historyConfirmationToDelete());
    m_checkForUpdates->setChecked(config.checkForUpdates());
    m_allowMultipleGuiInstances->setChecked(config.allowMultipleGuiInstances());
    m_showMagnifier->setChecked(config.showMagnifier());
    m_squareMagnifier->setChecked(config.squareMagnifier());

#if !defined(Q_OS_WIN)
    m_autoCloseIdleDaemon->setChecked(config.autoCloseIdleDaemon());
#endif

    m_predefinedColorPaletteLarge->setChecked(
      config.predefinedColorPaletteLarge());
    m_showStartupLaunchMessage->setChecked(config.showStartupLaunchMessage());
    m_screenshotPathFixedCheck->setChecked(config.savePathFixed());
    m_uploadHistoryMax->setValue(config.uploadHistoryMax());
    m_undoLimit->setValue(config.undoLimit());

    if (allowEmptySavePath || !config.savePath().isEmpty()) {
        m_savePath->setText(config.savePath());
    }
#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
    m_showTray->setChecked(!config.disabledTrayIcon());
#endif
}

void GeneralConf::updateComponents()
{
    _updateComponents(false);
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
    ConfigHandler().setShowDesktopNotification(checked);
}

void GeneralConf::checkForUpdatesChanged(bool checked)
{
    ConfigHandler().setCheckForUpdates(checked);
}

void GeneralConf::allowMultipleGuiInstancesChanged(bool checked)
{
    ConfigHandler().setAllowMultipleGuiInstances(checked);
}

void GeneralConf::autoCloseIdleDaemonChanged(bool checked)
{
    ConfigHandler().setAutoCloseIdleDaemon(checked);
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
    QString defaultFileName = QSettings().fileName();
    QString fileName =
      QFileDialog::getSaveFileName(this, tr("Save File"), defaultFileName);

    // Cancel button or target same as source
    if (fileName.isNull() || fileName == defaultFileName) {
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
        ConfigHandler().setDefaultSettings();
        _updateComponents(true);
    }
}

void GeneralConf::initScrollArea()
{
    m_scrollArea = new QScrollArea(this);
    m_layout->addWidget(m_scrollArea);

    auto* content = new QWidget(m_scrollArea);
    m_scrollArea->setWidget(content);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    content->setObjectName("content");
    m_scrollArea->setObjectName("scrollArea");
    m_scrollArea->setStyleSheet(
      "#content, #scrollArea { background: transparent; border: 0px; }");
    m_scrollAreaLayout = new QVBoxLayout(content);
    m_scrollAreaLayout->setContentsMargins(0, 0, 20, 0);
}

void GeneralConf::initShowHelp()
{
    m_helpMessage = new QCheckBox(tr("Show help message"), this);
    m_helpMessage->setToolTip(tr("Show the help message at the beginning "
                                 "in the capture mode"));
    m_scrollAreaLayout->addWidget(m_helpMessage);

    connect(
      m_helpMessage, &QCheckBox::clicked, this, &GeneralConf::showHelpChanged);
}

void GeneralConf::initShowSidePanelButton()
{
    m_sidePanelButton = new QCheckBox(tr("Show the side panel button"), this);
    m_sidePanelButton->setToolTip(
      tr("Show the side panel toggle button in the capture mode"));
    m_scrollAreaLayout->addWidget(m_sidePanelButton);

    connect(m_sidePanelButton,
            &QCheckBox::clicked,
            this,
            &GeneralConf::showSidePanelButtonChanged);
}

void GeneralConf::initShowDesktopNotification()
{
    m_sysNotifications = new QCheckBox(tr("Show desktop notifications"), this);
    m_sysNotifications->setToolTip(tr("Enable desktop notifications"));
    m_scrollAreaLayout->addWidget(m_sysNotifications);

    connect(m_sysNotifications,
            &QCheckBox::clicked,
            this,
            &GeneralConf::showDesktopNotificationChanged);
}

void GeneralConf::initShowTrayIcon()
{
#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
    m_showTray = new QCheckBox(tr("Show tray icon"), this);
    m_showTray->setToolTip(tr("Show icon in the system tray"));
    m_scrollAreaLayout->addWidget(m_showTray);

    connect(m_showTray, &QCheckBox::clicked, this, [](bool checked) {
        ConfigHandler().setDisabledTrayIcon(!checked);
    });
#endif
}

void GeneralConf::initHistoryConfirmationToDelete()
{
    m_historyConfirmationToDelete = new QCheckBox(
      tr("Confirmation required to delete screenshot from the latest uploads"),
      this);
    m_historyConfirmationToDelete->setToolTip(
      tr("Ask for confirmation to delete screenshot from the latest uploads"));
    m_scrollAreaLayout->addWidget(m_historyConfirmationToDelete);

    connect(m_historyConfirmationToDelete,
            &QCheckBox::clicked,
            this,
            &GeneralConf::historyConfirmationToDelete);
}

void GeneralConf::initConfigButtons()
{
    auto* buttonLayout = new QHBoxLayout();
    auto* box = new QGroupBox(tr("Configuration File"));
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

void GeneralConf::initCheckForUpdates()
{
    m_checkForUpdates = new QCheckBox(tr("Automatic check for updates"), this);
    m_checkForUpdates->setToolTip(tr("Check for updates automatically"));
    m_scrollAreaLayout->addWidget(m_checkForUpdates);

    connect(m_checkForUpdates,
            &QCheckBox::clicked,
            this,
            &GeneralConf::checkForUpdatesChanged);
}

void GeneralConf::initAllowMultipleGuiInstances()
{
    m_allowMultipleGuiInstances = new QCheckBox(
      tr("Allow multiple flameshot GUI instances simultaneously"), this);
    m_allowMultipleGuiInstances->setToolTip(tr(
      "This allows you to take screenshots of Flameshot itself for example"));
    m_scrollAreaLayout->addWidget(m_allowMultipleGuiInstances);
    connect(m_allowMultipleGuiInstances,
            &QCheckBox::clicked,
            this,
            &GeneralConf::allowMultipleGuiInstancesChanged);
}

void GeneralConf::initAutoCloseIdleDaemon()
{
    m_autoCloseIdleDaemon = new QCheckBox(
      tr("Automatically close daemon when it is not needed"), this);
    m_autoCloseIdleDaemon->setToolTip(
      tr("Automatically close daemon when it is not needed"));
    m_scrollAreaLayout->addWidget(m_autoCloseIdleDaemon);
    connect(m_autoCloseIdleDaemon,
            &QCheckBox::clicked,
            this,
            &GeneralConf::autoCloseIdleDaemonChanged);
}

void GeneralConf::initAutostart()
{
    m_autostart = new QCheckBox(tr("Launch at startup"), this);
    m_autostart->setToolTip(
      tr("Launch Flameshot daemon when computer is booted"));
    m_scrollAreaLayout->addWidget(m_autostart);

    connect(
      m_autostart, &QCheckBox::clicked, this, &GeneralConf::autostartChanged);
}

void GeneralConf::initShowStartupLaunchMessage()
{
    m_showStartupLaunchMessage =
      new QCheckBox(tr("Show welcome message on launch"), this);
    ConfigHandler config;
    m_showStartupLaunchMessage->setToolTip(
      tr("Show the welcome message box in the middle of the screen while "
         "taking a screenshot"));
    m_scrollAreaLayout->addWidget(m_showStartupLaunchMessage);

    connect(m_showStartupLaunchMessage, &QCheckBox::clicked, [](bool checked) {
        ConfigHandler().setShowStartupLaunchMessage(checked);
    });
}

void GeneralConf::initPredefinedColorPaletteLarge()
{
    m_predefinedColorPaletteLarge =
      new QCheckBox(tr("Use large predefined color palette"), this);
    m_predefinedColorPaletteLarge->setToolTip(
      tr("Use a large predefined color palette"));
    m_scrollAreaLayout->addWidget(m_predefinedColorPaletteLarge);

    connect(
      m_predefinedColorPaletteLarge, &QCheckBox::clicked, [](bool checked) {
          ConfigHandler().setPredefinedColorPaletteLarge(checked);
      });
}
void GeneralConf::initCopyOnDoubleClick()
{
    m_copyOnDoubleClick = new QCheckBox(tr("Copy on double click"), this);
    m_copyOnDoubleClick->setToolTip(tr("Enable Copy on Double Click"));
    m_scrollAreaLayout->addWidget(m_copyOnDoubleClick);

    connect(m_copyOnDoubleClick, &QCheckBox::clicked, [](bool checked) {
        ConfigHandler().setCopyOnDoubleClick(checked);
    });
}

void GeneralConf::initCopyAndCloseAfterUpload()
{
    m_copyAndCloseAfterUpload =
      new QCheckBox(tr("Copy URL after upload"), this);
    m_copyAndCloseAfterUpload->setToolTip(
      tr("Copy URL and close window after uploading was successful"));
    m_scrollAreaLayout->addWidget(m_copyAndCloseAfterUpload);

    connect(m_copyAndCloseAfterUpload, &QCheckBox::clicked, [](bool checked) {
        ConfigHandler().setCopyAndCloseAfterUpload(checked);
    });
}

void GeneralConf::initSaveAfterCopy()
{
    m_saveAfterCopy = new QCheckBox(tr("Save image after copy"), this);
    m_saveAfterCopy->setToolTip(
      tr("After copying the screenshot, save it to a file as well"));
    m_scrollAreaLayout->addWidget(m_saveAfterCopy);
    connect(m_saveAfterCopy,
            &QCheckBox::clicked,
            this,
            &GeneralConf::saveAfterCopyChanged);

    auto* box = new QGroupBox(tr("Save Path"));
    box->setFlat(true);
    m_layout->addWidget(box);

    auto* vboxLayout = new QVBoxLayout();
    box->setLayout(vboxLayout);

    auto* pathLayout = new QHBoxLayout();

    QString path = ConfigHandler().savePath();
    m_savePath = new QLineEdit(path, this);
    m_savePath->setDisabled(true);
    QString foreground = this->palette().windowText().color().name();
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
    connect(m_screenshotPathFixedCheck,
            SIGNAL(toggled(bool)),
            this,
            SLOT(togglePathFixed()));

    vboxLayout->addLayout(pathLayout);
    vboxLayout->addWidget(m_screenshotPathFixedCheck);

    auto* extensionLayout = new QHBoxLayout();

    extensionLayout->addWidget(
      new QLabel(tr("Preferred save file extension:")));
    m_setSaveAsFileExtension = new QComboBox(this);
    m_setSaveAsFileExtension->addItem("");

    QStringList imageFormatList;
    foreach (auto mimeType, QImageWriter::supportedImageFormats())
        imageFormatList.append(mimeType);

    m_setSaveAsFileExtension->addItems(imageFormatList);

    int currentIndex =
      m_setSaveAsFileExtension->findText(ConfigHandler().saveAsFileExtension());
    m_setSaveAsFileExtension->setCurrentIndex(currentIndex);

    connect(m_setSaveAsFileExtension,
            SIGNAL(currentTextChanged(QString)),
            this,
            SLOT(setSaveAsFileExtension(QString)));

    extensionLayout->addWidget(m_setSaveAsFileExtension);
    vboxLayout->addLayout(extensionLayout);
}

void GeneralConf::historyConfirmationToDelete(bool checked)
{
    ConfigHandler().setHistoryConfirmationToDelete(checked);
}

void GeneralConf::initUploadHistoryMax()
{
    auto* box = new QGroupBox(tr("Latest Uploads Max Size"));
    box->setFlat(true);
    m_layout->addWidget(box);

    auto* vboxLayout = new QVBoxLayout();
    box->setLayout(vboxLayout);

    m_uploadHistoryMax = new QSpinBox(this);
    m_uploadHistoryMax->setMaximum(50);
    QString foreground = this->palette().windowText().color().name();
    m_uploadHistoryMax->setStyleSheet(
      QStringLiteral("color: %1").arg(foreground));

    connect(m_uploadHistoryMax,
            SIGNAL(valueChanged(int)),
            this,
            SLOT(uploadHistoryMaxChanged(int)));
    vboxLayout->addWidget(m_uploadHistoryMax);
}

void GeneralConf::initUploadClientSecret()
{
    auto* box = new QGroupBox(tr("Imgur API Key"));
    box->setFlat(true);
    m_layout->addWidget(box);

    auto* vboxLayout = new QVBoxLayout();
    box->setLayout(vboxLayout);

    m_uploadClientKey = new QLineEdit(this);
    QString foreground = this->palette().windowText().color().name();
    m_uploadClientKey->setStyleSheet(
      QStringLiteral("color: %1").arg(foreground));
    m_uploadClientKey->setText(ConfigHandler().uploadClientSecret());
    connect(m_uploadClientKey,
            SIGNAL(editingFinished()),
            this,
            SLOT(uploadClientKeyEdited()));
    vboxLayout->addWidget(m_uploadClientKey);
}

void GeneralConf::uploadClientKeyEdited()
{
    ConfigHandler().setUploadClientSecret(m_uploadClientKey->text());
}

void GeneralConf::uploadHistoryMaxChanged(int max)
{
    ConfigHandler().setUploadHistoryMax(max);
}

void GeneralConf::initUndoLimit()
{
    auto* box = new QGroupBox(tr("Undo limit"));
    box->setFlat(true);
    m_layout->addWidget(box);

    auto* vboxLayout = new QVBoxLayout();
    box->setLayout(vboxLayout);

    m_undoLimit = new QSpinBox(this);
    m_undoLimit->setMinimum(1);
    m_undoLimit->setMaximum(999);
    QString foreground = this->palette().windowText().color().name();
    m_undoLimit->setStyleSheet(QStringLiteral("color: %1").arg(foreground));

    connect(m_undoLimit, SIGNAL(valueChanged(int)), this, SLOT(undoLimit(int)));

    vboxLayout->addWidget(m_undoLimit);
}

void GeneralConf::undoLimit(int limit)
{
    ConfigHandler().setUndoLimit(limit);
}

void GeneralConf::initUseJpgForClipboard()
{
    m_useJpgForClipboard =
      new QCheckBox(tr("Use JPG format for clipboard (PNG default)"), this);
    m_useJpgForClipboard->setToolTip(
      tr("Use JPG format for clipboard (PNG default)"));
    m_scrollAreaLayout->addWidget(m_useJpgForClipboard);

#if defined(Q_OS_MACOS)
    // FIXME - temporary fix to disable option for MacOS
    m_useJpgForClipboard->hide();
#endif
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
    path = chooseFolder(path);
    if (!path.isEmpty()) {
        m_savePath->setText(path);
        ConfigHandler().setSavePath(path);
    }
}

void GeneralConf::initCopyPathAfterSave()
{
    m_copyPathAfterSave = new QCheckBox(tr("Copy file path after save"), this);
    m_copyPathAfterSave->setToolTip(tr("Copy the file path to clipboard after "
                                       "the file is saved"));
    m_scrollAreaLayout->addWidget(m_copyPathAfterSave);
    connect(m_copyPathAfterSave, &QCheckBox::clicked, [](bool checked) {
        ConfigHandler().setCopyPathAfterSave(checked);
    });
}

void GeneralConf::initAntialiasingPinZoom()
{
    m_antialiasingPinZoom =
      new QCheckBox(tr("Anti-aliasing image when zoom the pinned image"), this);
    m_antialiasingPinZoom->setToolTip(
      tr("After zooming the pinned image, should the image get smoothened or "
         "stay pixelated"));
    m_scrollAreaLayout->addWidget(m_antialiasingPinZoom);
    connect(m_antialiasingPinZoom, &QCheckBox::clicked, [](bool checked) {
        ConfigHandler().setAntialiasingPinZoom(checked);
    });
}

void GeneralConf::initUploadWithoutConfirmation()
{
    m_uploadWithoutConfirmation =
      new QCheckBox(tr("Upload image without confirmation"), this);
    m_uploadWithoutConfirmation->setToolTip(
      tr("Upload image without confirmation"));
    m_scrollAreaLayout->addWidget(m_uploadWithoutConfirmation);
    connect(m_uploadWithoutConfirmation, &QCheckBox::clicked, [](bool checked) {
        ConfigHandler().setUploadWithoutConfirmation(checked);
    });
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

void GeneralConf::initShowMagnifier()
{
    m_showMagnifier = new QCheckBox(tr("Show magnifier"), this);
    m_showMagnifier->setToolTip(tr("Enable a magnifier while selecting the "
                                   "screenshot area"));

    m_scrollAreaLayout->addWidget(m_showMagnifier);
    connect(m_showMagnifier, &QCheckBox::clicked, [](bool checked) {
        ConfigHandler().setShowMagnifier(checked);
    });
}

void GeneralConf::initSquareMagnifier()
{
    m_squareMagnifier = new QCheckBox(tr("Square shaped magnifier"), this);
    m_squareMagnifier->setToolTip(tr("Make the magnifier to be square-shaped"));
    m_scrollAreaLayout->addWidget(m_squareMagnifier);
    connect(m_squareMagnifier, &QCheckBox::clicked, [](bool checked) {
        ConfigHandler().setSquareMagnifier(checked);
    });
}

void GeneralConf::togglePathFixed()
{
    ConfigHandler().setSavePathFixed(m_screenshotPathFixedCheck->isChecked());
}

void GeneralConf::setSaveAsFileExtension(QString extension)
{
    ConfigHandler().setSaveAsFileExtension(extension);
}

void GeneralConf::useJpgForClipboardChanged(bool checked)
{
    ConfigHandler().setUseJpgForClipboard(checked);
}
