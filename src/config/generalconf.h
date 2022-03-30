// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QScrollArea>
#include <QWidget>

class QVBoxLayout;
class QCheckBox;
class QPushButton;
class QLabel;
class QLineEdit;
class QSpinBox;
class QComboBox;

class GeneralConf : public QWidget
{
    Q_OBJECT
public:
    explicit GeneralConf(QWidget* parent = nullptr);

public slots:
    void updateComponents();

private slots:
    void showHelpChanged(bool checked);
    void showSidePanelButtonChanged(bool checked);
    void showDesktopNotificationChanged(bool checked);
    void checkForUpdatesChanged(bool checked);
    void allowMultipleGuiInstancesChanged(bool checked);
    void autoCloseIdleDaemonChanged(bool checked);
    void autostartChanged(bool checked);
    void historyConfirmationToDelete(bool checked);
    void uploadHistoryMaxChanged(int max);
    void undoLimit(int limit);
    void saveAfterCopyChanged(bool checked);
    void changeSavePath();
    void importConfiguration();
    void exportFileConfiguration();
    void resetConfiguration();
    void togglePathFixed();
    void uploadClientKeyEdited();
    void useJpgForClipboardChanged(bool checked);
    void setSaveAsFileExtension(QString extension);

private:
    const QString chooseFolder(const QString currentPath = "");

    void initAllowMultipleGuiInstances();
    void initAntialiasingPinZoom();
    void initAutoCloseIdleDaemon();
    void initAutostart();
    void initCheckForUpdates();
    void initConfigButtons();
    void initCopyAndCloseAfterUpload();
    void initCopyOnDoubleClick();
    void initCopyPathAfterSave();
    void initHistoryConfirmationToDelete();
    void initPredefinedColorPaletteLarge();
    void initSaveAfterCopy();
    void initScrollArea();
    void initShowDesktopNotification();
    void initShowHelp();
    void initShowMagnifier();
    void initShowSidePanelButton();
    void initShowStartupLaunchMessage();
    void initShowTrayIcon();
    void initSquareMagnifier();
    void initUndoLimit();
    void initUploadWithoutConfirmation();
    void initUseJpgForClipboard();
    void initUploadHistoryMax();
    void initUploadClientSecret();

    void _updateComponents(bool allowEmptySavePath);

    // class members
    QVBoxLayout* m_layout;
    QVBoxLayout* m_scrollAreaLayout;
    QScrollArea* m_scrollArea;
    QCheckBox* m_sysNotifications;
    QCheckBox* m_showTray;
    QCheckBox* m_helpMessage;
    QCheckBox* m_sidePanelButton;
    QCheckBox* m_checkForUpdates;
    QCheckBox* m_allowMultipleGuiInstances;
    QCheckBox* m_autoCloseIdleDaemon;
    QCheckBox* m_autostart;
    QCheckBox* m_showStartupLaunchMessage;
    QCheckBox* m_copyAndCloseAfterUpload;
    QCheckBox* m_copyPathAfterSave;
    QCheckBox* m_antialiasingPinZoom;
    QCheckBox* m_uploadWithoutConfirmation;
    QPushButton* m_importButton;
    QPushButton* m_exportButton;
    QPushButton* m_resetButton;
    QCheckBox* m_saveAfterCopy;
    QLineEdit* m_savePath;
    QLineEdit* m_uploadClientKey;
    QPushButton* m_changeSaveButton;
    QCheckBox* m_screenshotPathFixedCheck;
    QCheckBox* m_historyConfirmationToDelete;
    QCheckBox* m_useJpgForClipboard;
    QSpinBox* m_uploadHistoryMax;
    QSpinBox* m_undoLimit;
    QComboBox* m_setSaveAsFileExtension;
    QCheckBox* m_predefinedColorPaletteLarge;
    QCheckBox* m_showMagnifier;
    QCheckBox* m_squareMagnifier;
    QCheckBox* m_copyOnDoubleClick;
};
