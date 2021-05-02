// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QWidget>

class QVBoxLayout;
class QCheckBox;
class QPushButton;
class QLabel;
class QLineEdit;
class QSpinBox;

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
    void showTrayIconChanged(bool checked);
    void checkForUpdatesChanged(bool checked);
    void autostartChanged(bool checked);
    void historyConfirmationToDelete(bool checked);
    void uploadHistoryMaxSizeChanged(int max);
    void undoLimit(int limit);
    void saveAfterCopyChanged(bool checked);
    void changeSavePath();
    void importConfiguration();
    void exportFileConfiguration();
    void resetConfiguration();
    void togglePathFixed();
    void useJpgForClipboardChanged(bool checked);

private:
    const QString chooseFolder(const QString currentPath = "");

    void initShowHelp();
    void initShowSidePanelButton();
    void initShowDesktopNotification();
    void initShowTrayIcon();
    void initHistoryConfirmationToDelete();
    void initUploadHistoryMaxSize();
    void initUndoLimit();
    void initConfigButtons();
    void initCheckForUpdates();
    void initAutostart();
    void initShowStartupLaunchMessage();
    void initCopyAndCloseAfterUpload();
    void initSaveAfterCopy();
    void initCopyPathAfterSave();
    void initUseJpgForClipboard();

    void setActualFormData();

    // class members
    QVBoxLayout* m_layout;
    QCheckBox* m_sysNotifications;
    QCheckBox* m_showTray;
    QCheckBox* m_helpMessage;
    QCheckBox* m_sidePanelButton;
    QCheckBox* m_checkForUpdates;
    QCheckBox* m_autostart;
    QCheckBox* m_showStartupLaunchMessage;
    QCheckBox* m_copyAndCloseAfterUpload;
    QCheckBox* m_copyPathAfterSave;
    QPushButton* m_importButton;
    QPushButton* m_exportButton;
    QPushButton* m_resetButton;
    QCheckBox* m_saveAfterCopy;
    QLineEdit* m_savePath;
    QPushButton* m_changeSaveButton;
    QCheckBox* m_screenshotPathFixedCheck;
    QCheckBox* m_historyConfirmationToDelete;
    QCheckBox* m_useJpgForClipboard;
    QSpinBox* m_uploadHistoryMaxSize;
    QSpinBox* m_undoLimit;
};
