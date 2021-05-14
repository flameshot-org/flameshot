// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include "src/widgets/capture/capturetoolbutton.h"
#include <QSettings>
#include <QStringList>
#include <QVariant>
#include <QVector>

class ConfigHandler
{
public:
    explicit ConfigHandler();

    QVector<CaptureToolButton::ButtonType> getButtons();
    void setButtons(const QVector<CaptureToolButton::ButtonType>&);

    QVector<QColor> getUserColors();

    QString savePath();
    void setSavePath(const QString&);

    bool savePathFixed();
    void setSavePathFixed(bool);

    QColor uiMainColorValue();
    void setUIMainColor(const QColor&);

    QColor uiContrastColorValue();
    void setUIContrastColor(const QColor&);

    QColor drawColorValue();
    void setDrawColor(const QColor&);

    void setFontFamily(const QString&);
    const QString& fontFamily();

    bool showHelpValue();
    void setShowHelp(const bool);

    bool showSidePanelButtonValue();
    void setShowSidePanelButton(const bool);

    bool desktopNotificationValue();
    void setDesktopNotification(const bool);

    QString filenamePatternDefault();
    QString filenamePatternValue();
    void setFilenamePattern(const QString&);

    bool disabledTrayIconValue();
    void setDisabledTrayIcon(const bool);

    int drawThicknessValue();
    void setDrawThickness(const int);

    int drawFontSizeValue();
    void setDrawFontSize(const int);

    bool keepOpenAppLauncherValue();
    void setKeepOpenAppLauncher(const bool);

    bool checkForUpdates();
    void setCheckForUpdates(const bool);

    bool verifyLaunchFile();
    bool startupLaunchValue();
    void setStartupLaunch(const bool);

    bool showStartupLaunchMessage();
    void setShowStartupLaunchMessage(const bool);

    int contrastOpacityValue();
    void setContrastOpacity(const int);

    bool copyAndCloseAfterUploadEnabled();
    void setCopyAndCloseAfterUploadEnabled(const bool);

    bool historyConfirmationToDelete();
    void setHistoryConfirmationToDelete(const bool save);

    int uploadHistoryMaxSizeValue();
    void setUploadHistoryMaxSize(const int);

    bool saveAfterCopyValue();
    void setSaveAfterCopy(const bool);

    bool copyPathAfterSaveEnabled();
    void setCopyPathAfterSaveEnabled(const bool);

    bool useJpgForClipboard() const;
    void setUseJpgForClipboard(const bool);
    void setSaveAsFileExtension(const QString& extension);
    QString getSaveAsFileExtension();

    void setDefaultSettings();
    void setAllTheButtons();

    void setIgnoreUpdateToVersion(const QString& text);
    QString ignoreUpdateToVersion();

    void setUndoLimit(int value);
    int undoLimit();

    QVector<QStringList> shortcuts();
    void setShortcutsDefault();
    bool setShortcut(const QString&, const QString&);
    const QString& shortcut(const QString&);

    QString configFilePath() const;

    void setValue(const QString& group,
                  const QString& key,
                  const QVariant& value);
    QVariant& value(const QString& group, const QString& key);

private:
    QString m_strRes;
    QVariant m_varRes;
    QSettings m_settings;
    QVector<QStringList> m_shortcuts;

    bool normalizeButtons(QVector<int>&);

    QVector<CaptureToolButton::ButtonType> fromIntToButton(
      const QVector<int>& l);
    QVector<int> fromButtonToInt(
      const QVector<CaptureToolButton::ButtonType>& l);
};
