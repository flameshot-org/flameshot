// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include "src/widgets/capture/capturetoolbutton.h"
#include <QSettings>
#include <QStringList>
#include <QVariant>
#include <QVector>

class QFileSystemWatcher;
class ValueHandler;
template<class T>
class QSharedPointer;

class ConfigHandler : public QObject
{
    Q_OBJECT

public:
    explicit ConfigHandler();

    static ConfigHandler* getInstance();

    QList<CaptureToolButton::ButtonType> buttons();
    void setButtons(const QList<CaptureToolButton::ButtonType>&);

    QVector<QColor> userColors();

    QString savePath();
    void setSavePath(const QString&);

    bool savePathFixed();
    void setSavePathFixed(bool);

    QColor uiMainColor();
    void setUiMainColor(const QColor&);

    QColor contrastUiColor();
    void setContrastUiColor(const QColor&);

    QColor drawColor();
    void setDrawColor(const QColor&);

    QString fontFamily();
    void setFontFamily(const QString&);

    bool showHelp();
    void setShowHelp(const bool);

    bool showSidePanelButton();
    void setShowSidePanelButton(bool);

    bool showDesktopNotification();
    void setDesktopNotification(bool);

    QString filenamePatternDefault();
    QString filenamePattern();
    void setFilenamePattern(const QString&);

    bool disabledTrayIcon();
    void setDisabledTrayIcon(bool);

    int drawThickness();
    void setDrawThickness(const int);

    int drawFontSize();
    void setDrawFontSize(const int);

    bool keepOpenAppLauncher();
    void setKeepOpenAppLauncher(const bool);

    bool checkForUpdates();
    void setCheckForUpdates(const bool);

    bool verifyLaunchFile();
    bool startupLaunch();
    void setStartupLaunch(const bool);

    bool showStartupLaunchMessage();
    void setShowStartupLaunchMessage(const bool);

    int contrastOpacity();
    void setContrastOpacity(const int);

    bool copyAndCloseAfterUploadEnabled();
    void setCopyAndCloseAfterUploadEnabled(const bool);

    bool historyConfirmationToDelete();
    void setHistoryConfirmationToDelete(const bool save);

    int uploadHistoryMaxSize();
    void setUploadHistoryMaxSize(const int);

    bool saveAfterCopy();
    void setSaveAfterCopy(const bool);

    bool copyPathAfterSaveEnabled();
    void setCopyPathAfterSaveEnabled(const bool);

    bool useJpgForClipboard() const;
    void setUseJpgForClipboard(const bool);
    // TODO different than option name
    QString saveAsFileExtension();
    void setSaveAsFileExtension(const QString& extension);

    void setDefaultSettings();
    void setAllTheButtons();

    void setIgnoreUpdateToVersion(const QString& text);
    QString ignoreUpdateToVersion();

    void setUndoLimit(int value);
    int undoLimit();

    bool setShortcut(const QString&, const QString&);
    QString shortcut(const QString&);

    QString configFilePath() const;

    void setValue(const QString& key, const QVariant& value);
    QVariant value(const QString& key) const;
    bool contains(const QString& key) const;

    const QStringList& recognizedGeneralOptions() const;
    QStringList recognizedShortcutNames() const;
    QStringList keysFromGroup(const QString& group) const;
    bool isValidShortcutName(const QString& name) const;

    void checkAndHandleError() const;
    bool checkUnrecognizedSettings() const;
    bool checkShortcutConflicts() const;
    bool checkSemantics() const;
    void handleNewErrorState(bool error) const;
    bool hasError() const;
    QString errorMessage() const;
signals:
    void fileChanged() const;
    void error() const;
    void errorResolved() const;

private:
    QString m_strRes;
    QVariant m_varRes;
    mutable QSettings m_settings;
    QVector<QStringList> m_shortcuts;

    static bool m_hasError, m_errorCheckPending, m_skipNextErrorCheck;
    static QSharedPointer<QFileSystemWatcher> m_configWatcher;

    void ensureFileWatched() const;
    QSharedPointer<ValueHandler> valueHandler(const QString& key) const;
};
