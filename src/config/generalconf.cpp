// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "generalconf.h"
#include "utils/confighandler.h"

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
#include <QSpinBox>
#include <QStandardPaths>
#include <QStringDecoder>
#include <QVBoxLayout>

GeneralConf::GeneralConf(QWidget* parent)
  : QWidget(parent)
  , m_historyConfirmationToDelete(nullptr)
  , m_undoLimit(nullptr)
{
    m_layout = new QVBoxLayout(this);
    m_layout->setAlignment(Qt::AlignTop);
    m_layout->setSpacing(10);

    // Scroll area adapts the size of the content on small screens.
    // It must be initialized before the checkboxes.
    initScrollArea();

    {
        QVBoxLayout* outer = m_scrollAreaLayout;
        QVBoxLayout* group = pushGroupBox(tr("Startup"));
        m_scrollAreaLayout = group;
        initAutostart();
        initAutoCloseIdleDaemon();
        initShowTrayIcon();
        m_scrollAreaLayout = outer;
        // Forces this group's size hint to be recomputed right away, with
        // all 3 checkboxes already added - otherwise a QGroupBox
        // populated after being placed inside a resizable QScrollArea can
        // end up sized (and clipped) as if it only ever held its first
        // child.
        group->activate();
    }
    {
        QVBoxLayout* outer = m_scrollAreaLayout;
        QVBoxLayout* group = pushGroupBox(tr("Notifications & Behavior"));
        m_scrollAreaLayout = group;
        initShowDesktopNotification();
        initShowAbortNotification();
#if !defined(DISABLE_UPDATE_CHECKER)
        initCheckForUpdates();
#endif
        initShowStartupLaunchMessage();
        initShowQuitPrompt();
        initAllowMultipleGuiInstances();
        initSaveLastRegion();
        initShowHelp();
        initShowSidePanelButton();
        initUseJpgForClipboard();
        initCopyOnDoubleClick();
        m_scrollAreaLayout = outer;
        group->activate();
    }
    // Where screenshots end up is core to every capture, so the save
    // settings (which also carry JPEG Quality now, folded into the Save
    // Path box) rank above the general Options toggles below.
    initSaveAfterCopy();
    initSaveLocations();
    {
        QVBoxLayout* outer = m_scrollAreaLayout;
        QVBoxLayout* group = pushGroupBox(tr("Options"));
        m_scrollAreaLayout = group;
        initCopyPathAfterSave();
        initAntialiasingPinZoom();
        initInsecurePixelate();
#if !defined(Q_OS_MACOS)
        initCaptureActiveMonitor();
#endif
#if defined(Q_OS_MACOS)
        initUseNativeFullscreen();
#endif
#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
        initUseX11LegacyScreenshot();
#endif
#ifdef ENABLE_IMGUR
        initCopyAndCloseAfterUpload();
        initUploadWithoutConfirmation();
        initHistoryConfirmationToDelete();
#endif
        initPredefinedColorPaletteLarge();
        initShowMagnifier();
        initSquareMagnifier();
        initReverseArrow();
        m_scrollAreaLayout = outer;
        group->activate();
    }
    // Fine-tuning knobs that are set once and rarely revisited - lowest
    // priority, just above the Configuration File actions.
    initUndoLimit();
#ifdef ENABLE_IMGUR
    initUploadClientSecret();
#endif
    initShowSelectionGeometry();

    m_scrollAreaLayout->addStretch();

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
    m_abortNotifications->setChecked(config.showAbortNotification());
    m_autostart->setChecked(config.startupLaunch());
    m_saveAfterCopy->setChecked(config.saveAfterCopy());
    m_copyPathAfterSave->setChecked(config.copyPathAfterSave());
    m_antialiasingPinZoom->setChecked(config.antialiasingPinZoom());
    m_useJpgForClipboard->setChecked(config.useJpgForClipboard());
    m_copyOnDoubleClick->setChecked(config.copyOnDoubleClick());
#ifdef ENABLE_IMGUR
    m_uploadWithoutConfirmation->setChecked(config.uploadWithoutConfirmation());
    m_copyURLAfterUpload->setChecked(config.copyURLAfterUpload());
    m_showPostUploadDialog->setChecked(config.showPostUploadDialog());
    m_historyConfirmationToDelete->setChecked(
      config.historyConfirmationToDelete());

    m_uploadHistoryMax->setValue(config.uploadHistoryMax());
#endif
#if !defined(DISABLE_UPDATE_CHECKER)
    m_checkForUpdates->setChecked(config.checkForUpdates());
#endif
    m_allowMultipleGuiInstances->setChecked(config.allowMultipleGuiInstances());
    m_showMagnifier->setChecked(config.showMagnifier());
    m_squareMagnifier->setChecked(config.squareMagnifier());
    m_saveLastRegion->setChecked(config.saveLastRegion());
    m_reverseArrow->setChecked(config.reverseArrow());
    m_autoCloseIdleDaemon->setChecked(config.autoCloseIdleDaemon());
    m_predefinedColorPaletteLarge->setChecked(
      config.predefinedColorPaletteLarge());
    m_showStartupLaunchMessage->setChecked(config.showStartupLaunchMessage());
    m_showQuitPrompt->setChecked(config.showQuitPrompt());
    m_screenshotPathFixedCheck->setChecked(config.savePathFixed());
    m_undoLimit->setValue(config.undoLimit());

    if (allowEmptySavePath || !config.savePath().isEmpty()) {
        m_savePath->setText(config.savePath());
    }
    m_saveLocation1->setText(config.savePathLocation1());
    m_saveLocation2->setText(config.savePathLocation2());
    m_saveLocation3->setText(config.savePathLocation3());

    m_showTray->setChecked(!config.disabledTrayIcon());

#if !defined(Q_OS_MACOS)
    m_captureActiveMonitor->setChecked(config.captureActiveMonitor());
#endif
#if defined(Q_OS_MACOS)
    m_useNativeFullscreen->setChecked(config.useNativeFullscreen());
#endif
#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    m_useX11LegacyScreenshot->setChecked(config.useX11LegacyScreenshot());
#endif
}

void GeneralConf::updateComponents()
{
    _updateComponents(false);
}

void GeneralConf::saveLastRegion(bool checked)
{
    ConfigHandler().setSaveLastRegion(checked);
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

void GeneralConf::showAbortNotificationChanged(bool checked)
{
    ConfigHandler().setShowAbortNotification(checked);
}

#if !defined(DISABLE_UPDATE_CHECKER)
void GeneralConf::checkForUpdatesChanged(bool checked)
{
    ConfigHandler().setCheckForUpdates(checked);
}
#endif

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
    if (!file.open(QFile::ReadOnly)) {
        QMessageBox::about(this, tr("Error"), tr("Unable to read file."));
        return;
    }
    QStringDecoder decoder(QStringDecoder::System);
    QString text = decoder(file.readAll());
    file.close();

    QFile config(ConfigHandler().configFilePath());
    if (!config.open(QFile::WriteOnly)) {
        QMessageBox::about(this, tr("Error"), tr("Unable to write file."));
        return;
    }
    QStringEncoder encoder(QStringEncoder::System);
    config.write(encoder(text));
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
    // m_layout (this widget's own top-level layout) holds ONLY the scroll
    // area - every section (Startup, Save Path, Shortcuts locations, all
    // of it) is added to m_scrollAreaLayout below, not m_layout, so the
    // whole tab scrolls as one unit instead of the dialog growing to fit
    // all of it unconditionally.
    m_layout->addWidget(m_scrollArea);

    auto* content = new QWidget(m_scrollArea);
    m_scrollArea->setWidget(content);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    content->setObjectName("content");
    m_scrollArea->setObjectName("scrollArea");
    m_scrollArea->setStyleSheet(
      "#content, #scrollArea { background: transparent; border: 0px; }");
    m_scrollAreaLayout = new QVBoxLayout(content);
    m_scrollAreaLayout->setContentsMargins(0, 0, 20, 0);
    m_scrollAreaLayout->setSpacing(10);
}

QVBoxLayout* GeneralConf::pushGroupBox(const QString& title)
{
    auto* box = new QGroupBox(title);
    box->setFlat(true);
    auto* layout = new QVBoxLayout();
    layout->setSpacing(8);
    box->setLayout(layout);
    m_scrollAreaLayout->addWidget(box);
    return layout;
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

void GeneralConf::initSaveLastRegion()
{
    m_saveLastRegion = new QCheckBox(tr("Use last region for GUI mode"), this);
    m_saveLastRegion->setToolTip(
      tr("Use the last region as the default selection for the next screenshot "
         "in GUI mode"));
    m_scrollAreaLayout->addWidget(m_saveLastRegion);

    connect(m_saveLastRegion,
            &QCheckBox::clicked,
            this,
            &GeneralConf::saveLastRegion);
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

void GeneralConf::initShowAbortNotification()
{
    m_abortNotifications = new QCheckBox(tr("Show abort notifications"), this);
    m_abortNotifications->setToolTip(tr("Enable abort notifications"));
    m_scrollAreaLayout->addWidget(m_abortNotifications);

    connect(m_abortNotifications,
            &QCheckBox::clicked,
            this,
            &GeneralConf::showAbortNotificationChanged);
}

void GeneralConf::initShowTrayIcon()
{
    m_showTray = new QCheckBox(tr("Show tray icon"), this);
    m_showTray->setToolTip(tr("Show icon in the system tray"));
    m_scrollAreaLayout->addWidget(m_showTray);

    connect(m_showTray, &QCheckBox::clicked, this, [](bool checked) {
        ConfigHandler().setDisabledTrayIcon(!checked);
    });
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
    m_scrollAreaLayout->addWidget(box);

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

#if !defined(DISABLE_UPDATE_CHECKER)
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
#endif

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
      tr("Automatically unload from memory when it is not needed"), this);
    m_autoCloseIdleDaemon->setToolTip(tr(
      "Automatically close daemon (background process) when it is not needed"));
    m_scrollAreaLayout->addWidget(m_autoCloseIdleDaemon);
    connect(m_autoCloseIdleDaemon,
            &QCheckBox::clicked,
            this,
            &GeneralConf::autoCloseIdleDaemonChanged);
}

void GeneralConf::initAutostart()
{
    m_autostart = new QCheckBox(tr("Launch in background at startup"), this);
    m_autostart->setToolTip(tr(
      "Launch Flameshot daemon (background process) when computer is booted"));
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

void GeneralConf::initShowQuitPrompt()
{
    m_showQuitPrompt = new QCheckBox(tr("Ask before quit capture"), this);
    ConfigHandler config;
    m_showQuitPrompt->setToolTip(
      tr("Show the confirmation prompt before ESC quit"));
    m_scrollAreaLayout->addWidget(m_showQuitPrompt);

    connect(m_showQuitPrompt, &QCheckBox::clicked, [](bool checked) {
        ConfigHandler().setShowQuitPrompt(checked);
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
    m_copyOnDoubleClick->setToolTip(
      tr("Enable Copy to clipboard on Double Click"));
    m_scrollAreaLayout->addWidget(m_copyOnDoubleClick);

    connect(m_copyOnDoubleClick, &QCheckBox::clicked, [](bool checked) {
        ConfigHandler().setCopyOnDoubleClick(checked);
    });
}

void GeneralConf::initCopyAndCloseAfterUpload()
{
    m_copyURLAfterUpload = new QCheckBox(tr("Copy URL after upload"), this);
    m_copyURLAfterUpload->setToolTip(
      tr("Copy URL after uploading was successful"));
    m_scrollAreaLayout->addWidget(m_copyURLAfterUpload);

    connect(m_copyURLAfterUpload, &QCheckBox::clicked, [](bool checked) {
        ConfigHandler().setCopyURLAfterUpload(checked);
    });

    m_showPostUploadDialog =
      new QCheckBox(tr("Show upload result window"), this);
    m_showPostUploadDialog->setToolTip(
      tr("Open a window with the image and link after every upload, "
         "instead of just copying the link"));
    m_scrollAreaLayout->addWidget(m_showPostUploadDialog);

    connect(m_showPostUploadDialog, &QCheckBox::clicked, [](bool checked) {
        ConfigHandler().setShowPostUploadDialog(checked);
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
    m_scrollAreaLayout->addWidget(box);

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
            &QCheckBox::toggled,
            this,
            &GeneralConf::togglePathFixed);

    vboxLayout->addLayout(pathLayout);
    vboxLayout->addWidget(m_screenshotPathFixedCheck);

    auto* extensionLayout = new QHBoxLayout();

    extensionLayout->addWidget(
      new QLabel(tr("Preferred save file extension:")));
    m_setSaveAsFileExtension = new QComboBox(this);

    QStringList imageFormatList;
    for (const auto& mimeType : QImageWriter::supportedImageFormats())
        imageFormatList.append(mimeType);

    m_setSaveAsFileExtension->addItems(imageFormatList);

    int currentIndex =
      m_setSaveAsFileExtension->findText(ConfigHandler().saveAsFileExtension());
    m_setSaveAsFileExtension->setCurrentIndex(currentIndex);

    connect(m_setSaveAsFileExtension,
            &QComboBox::currentTextChanged,
            this,
            &GeneralConf::setSaveAsFileExtension);

    extensionLayout->addWidget(m_setSaveAsFileExtension);
    vboxLayout->addLayout(extensionLayout);

    // JPEG Quality only matters for the save/upload/clipboard file format
    // set above - nest it in this same box instead of leaving it as an
    // unrelated floating row elsewhere in the tab.
    QVBoxLayout* outer = m_scrollAreaLayout;
    m_scrollAreaLayout = vboxLayout;
    initJpegQuality();
    m_scrollAreaLayout = outer;
}

void GeneralConf::historyConfirmationToDelete(bool checked)
{
    ConfigHandler().setHistoryConfirmationToDelete(checked);
}

void GeneralConf::initUploadClientSecret()
{
    auto* box = new QGroupBox(tr("Imgur Application Client ID"));
    box->setFlat(true);
    m_scrollAreaLayout->addWidget(box);

    auto* vboxLayout = new QVBoxLayout();
    box->setLayout(vboxLayout);

    m_uploadClientKey = new QLineEdit(this);
    QString foreground = this->palette().windowText().color().name();
    m_uploadClientKey->setStyleSheet(
      QStringLiteral("color: %1").arg(foreground));
    m_uploadClientKey->setText(ConfigHandler().uploadClientSecret());
    connect(m_uploadClientKey,
            &QLineEdit::editingFinished,
            this,
            &GeneralConf::uploadClientKeyEdited);
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
    // One full-width titled box, matching every other section in this
    // tab, instead of two mini boxes hugging their content on the left
    // with dead space to the right.
    auto* box = new QGroupBox(tr("Limits"));
    box->setFlat(true);
    m_scrollAreaLayout->addWidget(box);

    auto* vboxLayout = new QVBoxLayout();
    box->setLayout(vboxLayout);
    QString foreground = this->palette().windowText().color().name();

    auto* undoRow = new QHBoxLayout();
    undoRow->addWidget(new QLabel(tr("Undo limit")));
    m_undoLimit = new QSpinBox(this);
    m_undoLimit->setRange(1, 999);
    m_undoLimit->setStyleSheet(QStringLiteral("color: %1").arg(foreground));
    undoRow->addWidget(m_undoLimit);
    vboxLayout->addLayout(undoRow);
    connect(m_undoLimit,
            static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this,
            &GeneralConf::undoLimit);

#ifdef ENABLE_IMGUR
    auto* uploadRow = new QHBoxLayout();
    uploadRow->addWidget(new QLabel(tr("Latest Uploads Max Size")));
    m_uploadHistoryMax = new QSpinBox(this);
    m_uploadHistoryMax->setMaximum(50);
    m_uploadHistoryMax->setStyleSheet(
      QStringLiteral("color: %1").arg(foreground));
    uploadRow->addWidget(m_uploadHistoryMax);
    vboxLayout->addLayout(uploadRow);
    connect(m_uploadHistoryMax,
            static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this,
            &GeneralConf::uploadHistoryMaxChanged);
#endif

    vboxLayout->addStretch();
}

void GeneralConf::undoLimit(int limit)
{
    ConfigHandler().setUndoLimit(limit);
}

void GeneralConf::initUseJpgForClipboard()
{
    m_useJpgForClipboard =
      new QCheckBox(tr("Use JPG format for clipboard (PNG default)"), this);

#ifdef Q_OS_WIN
    ConfigHandler().setUseJpgForClipboard(false);
    m_useJpgForClipboard->setVisible(false);
#else
    m_useJpgForClipboard->setToolTip(
      tr("Use lossy JPG format for clipboard (lossless PNG default)"));
    connect(m_useJpgForClipboard,
            &QCheckBox::clicked,
            this,
            &GeneralConf::useJpgForClipboardChanged);
#endif

    m_scrollAreaLayout->addWidget(m_useJpgForClipboard);
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

// 3 extra save destinations, each exposed as its own tool
// (TYPE_SAVE_LOCATION_1/2/3) so a hotkey can be bound to it from the
// Shortcuts tab, saving straight to that folder with no dialog.
void GeneralConf::initSaveLocations()
{
    auto* box = new QGroupBox(tr("Save Path Locations (hotkey-bindable)"));
    box->setFlat(true);
    m_scrollAreaLayout->addWidget(box);

    auto* vboxLayout = new QVBoxLayout();
    box->setLayout(vboxLayout);

    QString foreground = this->palette().windowText().color().name();
    ConfigHandler config;

    QLineEdit** lineEdits[3] = { &m_saveLocation1,
                                 &m_saveLocation2,
                                 &m_saveLocation3 };
    QPushButton** buttons[3] = { &m_changeSaveLocation1Button,
                                 &m_changeSaveLocation2Button,
                                 &m_changeSaveLocation3Button };
    QString savedPaths[3] = { config.savePathLocation1(),
                              config.savePathLocation2(),
                              config.savePathLocation3() };

    for (int i = 0; i < 3; ++i) {
        auto* rowLayout = new QHBoxLayout();
        rowLayout->addWidget(new QLabel(tr("Location %1:").arg(i + 1)));

        auto* lineEdit = new QLineEdit(savedPaths[i], this);
        lineEdit->setDisabled(true);
        lineEdit->setStyleSheet(QStringLiteral("color: %1").arg(foreground));
        rowLayout->addWidget(lineEdit);
        *lineEdits[i] = lineEdit;

        auto* button = new QPushButton(tr("Change..."), this);
        rowLayout->addWidget(button);
        *buttons[i] = button;

        int location = i + 1;
        connect(button, &QPushButton::clicked, this, [this, location]() {
            changeSaveLocation(location);
        });

        vboxLayout->addLayout(rowLayout);
    }
}

void GeneralConf::changeSaveLocation(int location)
{
    ConfigHandler config;
    QLineEdit* lineEdit = nullptr;
    QString current;
    switch (location) {
        case 1:
            lineEdit = m_saveLocation1;
            current = config.savePathLocation1();
            break;
        case 2:
            lineEdit = m_saveLocation2;
            current = config.savePathLocation2();
            break;
        default:
            lineEdit = m_saveLocation3;
            current = config.savePathLocation3();
            break;
    }

    QString path = chooseFolder(current);
    if (path.isEmpty()) {
        return;
    }

    lineEdit->setText(path);
    switch (location) {
        case 1:
            config.setSavePathLocation1(path);
            break;
        case 2:
            config.setSavePathLocation2(path);
            break;
        default:
            config.setSavePathLocation3(path);
            break;
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

const QString GeneralConf::chooseFolder(const QString& pathDefault)
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

    if (!QFileInfo(path).isWritable()) {
        QMessageBox::about(
          this, tr("Error"), tr("Unable to write to directory."));
        return QString();
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

void GeneralConf::initShowSelectionGeometry()
{
    auto* box = new QGroupBox(tr("Selection Geometry Display"));
    box->setFlat(true);
    m_scrollAreaLayout->addWidget(box);

    auto* vboxLayout = new QVBoxLayout();
    box->setLayout(vboxLayout);

    auto* tobox = new QHBoxLayout();
    int timeout =
      ConfigHandler().value("showSelectionGeometryHideTime").toInt();
    m_xywhTimeout = new QSpinBox();
    m_xywhTimeout->setRange(0, INT_MAX);
    m_xywhTimeout->setToolTip(
      tr("Milliseconds before geometry display hides; 0 means do not hide"));
    m_xywhTimeout->setValue(timeout);
    tobox->addWidget(m_xywhTimeout);
    tobox->addWidget(new QLabel(tr("Set geometry display timeout (ms)")));
    vboxLayout->addLayout(tobox);
    connect(m_xywhTimeout,
            static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this,
            &GeneralConf::setSelGeoHideTime);

    auto* selGeoLayout = new QHBoxLayout();
    selGeoLayout->addWidget(new QLabel(tr("Display Location")));
    m_selectGeometryLocation = new QComboBox(this);

    m_selectGeometryLocation->addItem(tr("None"), GeneralConf::xywh_none);
    m_selectGeometryLocation->addItem(tr("Top Left"),
                                      GeneralConf::xywh_top_left);
    m_selectGeometryLocation->addItem(tr("Top Right"),
                                      GeneralConf::xywh_top_right);
    m_selectGeometryLocation->addItem(tr("Bottom Left"),
                                      GeneralConf::xywh_bottom_left);
    m_selectGeometryLocation->addItem(tr("Bottom Right"),
                                      GeneralConf::xywh_bottom_right);
    m_selectGeometryLocation->addItem(tr("Center"), GeneralConf::xywh_center);

    // pick up int from config and use findData
    int pos = ConfigHandler().value("showSelectionGeometry").toInt();
    m_selectGeometryLocation->setCurrentIndex(
      m_selectGeometryLocation->findData(pos));

    connect(
      m_selectGeometryLocation,
      static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
      this,
      &GeneralConf::setGeometryLocation);

    selGeoLayout->addWidget(m_selectGeometryLocation);
    vboxLayout->addLayout(selGeoLayout);
    vboxLayout->addStretch();
}

void GeneralConf::initJpegQuality()
{
    auto* tobox = new QHBoxLayout();

    int quality = ConfigHandler().value("jpegQuality").toInt();
    m_jpegQuality = new QSpinBox();
    m_jpegQuality->setRange(0, 100);
    m_jpegQuality->setToolTip(tr("Quality range of 0-100; Higher number is "
                                 "better quality and larger file size"));
    m_jpegQuality->setValue(quality);
    tobox->addWidget(m_jpegQuality);
    tobox->addWidget(new QLabel(tr("JPEG Quality")));

    m_scrollAreaLayout->addLayout(tobox);
    connect(m_jpegQuality,
            static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this,
            &GeneralConf::setJpegQuality);
}

void GeneralConf::initReverseArrow()
{
    m_reverseArrow = new QCheckBox(tr("Reverse arrow"), this);
    m_reverseArrow->setToolTip(tr("Draw the arrow head first"));
    m_scrollAreaLayout->addWidget(m_reverseArrow);

    connect(
      m_reverseArrow, &QCheckBox::clicked, this, &GeneralConf::setReverseArrow);
}

void GeneralConf::initInsecurePixelate()
{
    m_insecurePixelate = new QCheckBox(tr("Insecure Pixelate"), this);
    m_insecurePixelate->setToolTip(
      tr("Draw the pixelation effect in an insecure but more asethetic way."));
    m_insecurePixelate->setChecked(ConfigHandler().insecurePixelate());
    m_scrollAreaLayout->addWidget(m_insecurePixelate);

    connect(m_insecurePixelate,
            &QCheckBox::clicked,
            this,
            &GeneralConf::setInsecurePixelate);
}

void GeneralConf::setSelGeoHideTime(int v)
{
    ConfigHandler().setValue("showSelectionGeometryHideTime", v);
}

void GeneralConf::setJpegQuality(int v)
{
    ConfigHandler().setJpegQuality(v);
}

void GeneralConf::setGeometryLocation(int index)
{
    ConfigHandler().setValue("showSelectionGeometry",
                             m_selectGeometryLocation->itemData(index));
}

void GeneralConf::togglePathFixed()
{
    ConfigHandler().setSavePathFixed(m_screenshotPathFixedCheck->isChecked());
}

void GeneralConf::setSaveAsFileExtension(const QString& extension)
{
    ConfigHandler().setSaveAsFileExtension(extension);
}

void GeneralConf::useJpgForClipboardChanged(bool checked)
{
    ConfigHandler().setUseJpgForClipboard(checked);
}

void GeneralConf::setReverseArrow(bool checked)
{
    ConfigHandler().setReverseArrow(checked);
}

void GeneralConf::setInsecurePixelate(bool checked)
{
    ConfigHandler().setInsecurePixelate(checked);
}

#if !defined(Q_OS_MACOS)
void GeneralConf::initCaptureActiveMonitor()
{
    m_captureActiveMonitor = new QCheckBox(
      tr("Capture active monitor in X11 and Windows (skip monitor selection)"),
      this);
    m_captureActiveMonitor->setToolTip(
      tr("Automatically capture the monitor where the cursor is located "
         "instead of showing the monitor selection dialog. "
         "This feature is not supported on macOS and Wayland."));
    m_scrollAreaLayout->addWidget(m_captureActiveMonitor);

    connect(m_captureActiveMonitor,
            &QCheckBox::clicked,
            this,
            &GeneralConf::captureActiveMonitorChanged);
}

void GeneralConf::captureActiveMonitorChanged(bool checked)
{
    ConfigHandler().setCaptureActiveMonitor(checked);
}
#endif

#if defined(Q_OS_MACOS)
void GeneralConf::initUseNativeFullscreen()
{
    m_useNativeFullscreen =
      new QCheckBox(tr("Use native fullscreen for capture overlay"), this);
    m_useNativeFullscreen->setToolTip(
      tr("Use macOS native fullscreen mode for the capture overlay. "
         "When disabled (default), the overlay avoids the fullscreen "
         "desktop animation."));
    m_scrollAreaLayout->addWidget(m_useNativeFullscreen);

    connect(m_useNativeFullscreen,
            &QCheckBox::clicked,
            this,
            &GeneralConf::useNativeFullscreenChanged);
}

void GeneralConf::useNativeFullscreenChanged(bool checked)
{
    ConfigHandler().setUseNativeFullscreen(checked);
}
#endif

#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
void GeneralConf::initUseX11LegacyScreenshot()
{
    m_useX11LegacyScreenshot =
      new QCheckBox(tr("Use legacy X11 screenshot method"), this);
    m_useX11LegacyScreenshot->setToolTip(
      tr("Bypass the freedesktop portal and use Qt's native X11 screen "
         "capture. Enable this if your window manager lacks "
         "xdg-desktop-portal (e.g. xmonad, i3). "
         "Only effective on X11; ignored on Wayland."));
    m_scrollAreaLayout->addWidget(m_useX11LegacyScreenshot);

    connect(m_useX11LegacyScreenshot,
            &QCheckBox::clicked,
            this,
            &GeneralConf::useX11LegacyScreenshotChanged);
}

void GeneralConf::useX11LegacyScreenshotChanged(bool checked)
{
    ConfigHandler().setUseX11LegacyScreenshot(checked);
}
#endif
