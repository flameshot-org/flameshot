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

#pragma once

#include <QWidget>

class QVBoxLayout;
class QCheckBox;
class QPushButton;
class QLabel;
class QLineEdit;

class GeneneralConf : public QWidget
{
    Q_OBJECT
public:
    explicit GeneneralConf(QWidget* parent = nullptr);

public slots:
    void updateComponents();

private slots:
    void showHelpChanged(bool checked);
    void showSidePanelButtonChanged(bool checked);
    void showDesktopNotificationChanged(bool checked);
    void showTrayIconChanged(bool checked);
    void autostartChanged(bool checked);
    void saveAfterCopyChanged(bool checked);
    void changeSavePath();
    void importConfiguration();
    void exportFileConfiguration();
    void resetConfiguration();
    void togglePathFixed();

private:
    const QString chooseFolder(const QString currentPath = "");

    void initShowHelp();
    void initShowSidePanelButton();
    void initShowDesktopNotification();
    void initShowTrayIcon();
    void initConfingButtons();
    void initAutostart();
    void initSaveAfterCopy();

    // class members
    QVBoxLayout* m_layout;
    QCheckBox* m_sysNotifications;
    QCheckBox* m_showTray;
    QCheckBox* m_helpMessage;
    QCheckBox* m_sidePanelButton;
    QCheckBox* m_autostart;
    QPushButton* m_importButton;
    QPushButton* m_exportButton;
    QPushButton* m_resetButton;
    QCheckBox* m_saveAfterCopy;
    QLineEdit* m_savePath;
    QPushButton* m_changeSaveButton;
    QCheckBox* m_screenshotPathFixedCheck;
};
