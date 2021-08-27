// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "confighandler.h"
#include "src/tools/capturetool.h"
#include "src/utils/configshortcuts.h"
#include "systemnotification.h"
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileSystemWatcher>
#include <QKeySequence>
#include <QStandardPaths>
#include <algorithm>
#if defined(Q_OS_MACOS)
#include <QProcess>
#endif

bool ConfigHandler::m_hasError = false;
bool ConfigHandler::m_errorCheckPending = false;
QSharedPointer<QFileSystemWatcher> ConfigHandler::m_configWatcher;

ConfigHandler::ConfigHandler()
{
    m_settings.setDefaultFormat(QSettings::IniFormat);

    if (m_configWatcher == nullptr && qApp != nullptr) {
        // check for error on initial call
        checkAndHandleError();
        // check for error every time the file changes
        m_configWatcher.reset(new QFileSystemWatcher());
        ensureFileWatched();
        QObject::connect(m_configWatcher.get(),
                         &QFileSystemWatcher::fileChanged,
                         [](const QString& fileName) {
                             ConfigHandler().checkAndHandleError();
                             if (!QFile(fileName).exists()) {
                                 // File watcher stops watching a deleted file.
                                 // Next time the config is accessed, force it
                                 // to check for errors.
                                 m_errorCheckPending = true;
                             } else {
                                 m_configWatcher->addPath(fileName);
                             }
                         });
    }
}

QVector<CaptureToolButton::ButtonType> ConfigHandler::getButtons()
{
    QVector<CaptureToolButton::ButtonType> buttons;
    if (contains(QStringLiteral("buttons"))) {
        // TODO: remove toList in v1.0
        QVector<int> buttonsInt =
          value(QStringLiteral("buttons")).value<QList<int>>().toVector();
        bool modified = normalizeButtons(buttonsInt);
        if (modified) {
            setValue(QStringLiteral("buttons"),
                     QVariant::fromValue(buttonsInt.toList()));
        }
        buttons = fromIntToButton(buttonsInt);
    } else {
        // Default tools
        buttons = CaptureToolButton::getIterableButtonTypes();
        buttons.removeOne(CaptureToolButton::TYPE_SIZEDECREASE);
        buttons.removeOne(CaptureToolButton::TYPE_SIZEINCREASE);
    }

    using bt = CaptureToolButton::ButtonType;
    std::sort(buttons.begin(), buttons.end(), [](bt a, bt b) {
        return CaptureToolButton::getPriorityByButton(a) <
               CaptureToolButton::getPriorityByButton(b);
    });
    return buttons;
}

void ConfigHandler::setButtons(
  const QVector<CaptureToolButton::ButtonType>& buttons)
{
    QVector<int> l = fromButtonToInt(buttons);
    normalizeButtons(l);
    // TODO: remove toList in v1.0
    setValue(QStringLiteral("buttons"), QVariant::fromValue(l.toList()));
}

QVector<QColor> ConfigHandler::getUserColors()
{
    QVector<QColor> colors;
    const QVector<QColor>& defaultColors = {
        Qt::darkRed, Qt::red,  Qt::yellow,  Qt::green,       Qt::darkGreen,
        Qt::cyan,    Qt::blue, Qt::magenta, Qt::darkMagenta, QColor()
    };

    if (contains(QStringLiteral("userColors"))) {
        for (const QString& hex :
             value(QStringLiteral("userColors")).toStringList()) {
            if (QColor::isValidColor(hex)) {
                colors.append(QColor(hex));
            } else if (hex == QStringLiteral("picker")) {
                colors.append(QColor());
            }
        }

        if (colors.isEmpty()) {
            colors = defaultColors;
        }
    } else {
        colors = defaultColors;
    }

    return colors;
}

QString ConfigHandler::savePath()
{
    return value(QStringLiteral("savePath")).toString();
}

void ConfigHandler::setSavePath(const QString& savePath)
{
    setValue(QStringLiteral("savePath"), savePath);
}

bool ConfigHandler::savePathFixed()
{
    if (!contains(QStringLiteral("savePathFixed"))) {
        setValue(QStringLiteral("savePathFixed"), false);
    }
    return value(QStringLiteral("savePathFixed")).toBool();
}

void ConfigHandler::setSavePathFixed(bool savePathFixed)
{
    setValue(QStringLiteral("savePathFixed"), savePathFixed);
}

QColor ConfigHandler::uiMainColorValue()
{
    QColor res = QColor(116, 0, 150);

    if (contains(QStringLiteral("uiColor"))) {
        QString hex = value(QStringLiteral("uiColor")).toString();

        if (QColor::isValidColor(hex)) {
            res = QColor(hex);
        }
    }
    return res;
}

void ConfigHandler::setUIMainColor(const QColor& c)
{
    setValue(QStringLiteral("uiColor"), c.name());
}

QColor ConfigHandler::uiContrastColorValue()
{
    QColor res = QColor(39, 0, 50);

    if (contains(QStringLiteral("contrastUiColor"))) {
        QString hex = value(QStringLiteral("contrastUiColor")).toString();

        if (QColor::isValidColor(hex)) {
            res = QColor(hex);
        }
    }

    return res;
}

void ConfigHandler::setUIContrastColor(const QColor& c)
{
    setValue(QStringLiteral("contrastUiColor"), c.name());
}

QColor ConfigHandler::drawColorValue()
{
    QColor res(Qt::red);

    if (contains(QStringLiteral("drawColor"))) {
        QString hex = value(QStringLiteral("drawColor")).toString();

        if (QColor::isValidColor(hex)) {
            res = QColor(hex);
        }
    }

    return res;
}

void ConfigHandler::setDrawColor(const QColor& c)
{
    setValue(QStringLiteral("drawColor"), c.name());
}

void ConfigHandler::setFontFamily(const QString& fontFamily)
{
    setValue(QStringLiteral("fontFamily"), fontFamily);
}

const QString& ConfigHandler::fontFamily()
{
    m_strRes = value(QStringLiteral("fontFamily"), QString()).toString();
    return m_strRes;
}

bool ConfigHandler::showHelpValue()
{
    return value(QStringLiteral("showHelp"), true).toBool();
}

void ConfigHandler::setShowHelp(const bool showHelp)
{
    setValue(QStringLiteral("showHelp"), showHelp);
}

bool ConfigHandler::showSidePanelButtonValue()
{
    return value(QStringLiteral("showSidePanelButton"), true).toBool();
}

void ConfigHandler::setShowSidePanelButton(const bool showSidePanelButton)
{
    setValue(QStringLiteral("showSidePanelButton"), showSidePanelButton);
}

void ConfigHandler::setIgnoreUpdateToVersion(const QString& text)
{
    setValue(QStringLiteral("ignoreUpdateToVersion"), text);
}

QString ConfigHandler::ignoreUpdateToVersion()
{
    return value(QStringLiteral("ignoreUpdateToVersion")).toString();
}

void ConfigHandler::setUndoLimit(int value)
{
    setValue(QStringLiteral("undoLimit"), value);
}

int ConfigHandler::undoLimit()
{
    int val = value(QStringLiteral("undoLimit"), 100).toInt();
    return qBound(1, val, 999);
}

bool ConfigHandler::desktopNotificationValue()
{
    return value(QStringLiteral("showDesktopNotification"), true).toBool();
}

void ConfigHandler::setDesktopNotification(const bool showDesktopNotification)
{
    setValue(QStringLiteral("showDesktopNotification"),
             showDesktopNotification);
}

QString ConfigHandler::filenamePatternDefault()
{
    m_strRes = QLatin1String("%F_%H-%M");
    return m_strRes;
}

QString ConfigHandler::filenamePatternValue()
{
    m_strRes = value(QStringLiteral("filenamePattern")).toString();
    if (m_strRes.isEmpty()) {
        m_strRes = filenamePatternDefault();
    }
    return m_strRes;
}

void ConfigHandler::setFilenamePattern(const QString& pattern)
{
    return setValue(QStringLiteral("filenamePattern"), pattern);
}

bool ConfigHandler::disabledTrayIconValue()
{
    return value(QStringLiteral("disabledTrayIcon"), false).toBool();
}

void ConfigHandler::setDisabledTrayIcon(const bool disabledTrayIcon)
{
    setValue(QStringLiteral("disabledTrayIcon"), disabledTrayIcon);
}

int ConfigHandler::drawThicknessValue()
{
    return value(QStringLiteral("drawThickness"), 3).toInt();
}

void ConfigHandler::setDrawThickness(const int thickness)
{
    setValue(QStringLiteral("drawThickness"), thickness);
}

int ConfigHandler::drawFontSizeValue()
{
    return value(QStringLiteral("drawFontSize"), 8).toInt();
}

void ConfigHandler::setDrawFontSize(const int fontSize)
{
    setValue(QStringLiteral("drawFontSize"), fontSize);
}

bool ConfigHandler::keepOpenAppLauncherValue()
{
    return value(QStringLiteral("keepOpenAppLauncher")).toBool();
}

void ConfigHandler::setKeepOpenAppLauncher(const bool keepOpen)
{
    setValue(QStringLiteral("keepOpenAppLauncher"), keepOpen);
}

bool ConfigHandler::checkForUpdates()
{
    return value(QStringLiteral("checkForUpdates"), true).toBool();
}

void ConfigHandler::setCheckForUpdates(const bool checkForUpdates)
{
    setValue(QStringLiteral("checkForUpdates"), checkForUpdates);
}

bool ConfigHandler::startupLaunchValue()
{
#if defined(Q_OS_MACOS)
    bool res = false;
#else
    bool res = true;
#endif
    res = value(QStringLiteral("startupLaunch"), res).toBool();
    if (res != verifyLaunchFile()) {
        setStartupLaunch(res);
    }
    return res;
}

bool ConfigHandler::verifyLaunchFile()
{
#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
    QString path = QStandardPaths::locate(QStandardPaths::GenericConfigLocation,
                                          "autostart/",
                                          QStandardPaths::LocateDirectory) +
                   "Flameshot.desktop";
    bool res = QFile(path).exists();
#elif defined(Q_OS_WIN)
    QSettings bootUpSettings(
      "HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
      QSettings::NativeFormat);
    bool res =
      bootUpSettings.value("Flameshot").toString() ==
      QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
#endif
    return res;
}

void ConfigHandler::setStartupLaunch(const bool start)
{
    if (start == value(QStringLiteral("startupLaunch")).toBool()) {
        return;
    }
    setValue(QStringLiteral("startupLaunch"), start);
#if defined(Q_OS_MACOS)
    /* TODO - there should be more correct way via API, but didn't find it
     without extra dependencies, there should be something like that:
     https://stackoverflow.com/questions/3358410/programmatically-run-at-startup-on-mac-os-x
     But files with this features differs on different MacOS versions and it
     doesn't work not on a BigSur at lease.
     */
    QProcess process;
    if (start) {
        process.start("osascript",
                      QStringList()
                        << "-e"
                        << "tell application \"System Events\" to make login "
                           "item at end with properties {name: "
                           "\"Flameshot\",path:\"/Applications/"
                           "flameshot.app\", hidden:false}");
    } else {
        process.start("osascript",
                      QStringList() << "-e"
                                    << "tell application \"System Events\" to "
                                       "delete login item \"Flameshot\"");
    }
    if (!process.waitForFinished()) {
        qWarning() << "Login items is changed. " << process.errorString();
    } else {
        qWarning() << "Unable to change login items, error:"
                   << process.readAll();
    }
#elif defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
    QString path = QStandardPaths::locate(QStandardPaths::GenericConfigLocation,
                                          "autostart/",
                                          QStandardPaths::LocateDirectory);
    QDir autostartDir(path);
    if (!autostartDir.exists()) {
        autostartDir.mkpath(".");
    }

    QFile file(path + "Flameshot.desktop");
    if (start) {
        if (file.open(QIODevice::WriteOnly)) {
            QByteArray data("[Desktop Entry]\nName=flameshot\nIcon=flameshot"
                            "\nExec=flameshot\nTerminal=false\nType=Application"
                            "\nX-GNOME-Autostart-enabled=true\n");
            file.write(data);
        }
    } else {
        file.remove();
    }
#elif defined(Q_OS_WIN)
    QSettings bootUpSettings(
      "HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
      QSettings::NativeFormat);
    // set workdir for flameshot on startup
    QSettings bootUpPath(
      "HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App "
      "Paths",
      QSettings::NativeFormat);
    if (start) {
        QString app_path =
          QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
        bootUpSettings.setValue("Flameshot", app_path);

        // set application workdir
        bootUpPath.beginGroup("flameshot.exe");
        bootUpPath.setValue("Path", QCoreApplication::applicationDirPath());
        bootUpPath.endGroup();

    } else {
        bootUpSettings.remove("Flameshot");

        // remove application workdir
        bootUpPath.beginGroup("flameshot.exe");
        bootUpPath.remove("");
        bootUpPath.endGroup();
    }
#endif
}

bool ConfigHandler::showStartupLaunchMessage()
{
    if (!contains(QStringLiteral("showStartupLaunchMessage"))) {
        setValue(QStringLiteral("showStartupLaunchMessage"), true);
    }
    return value(QStringLiteral("showStartupLaunchMessage")).toBool();
}

void ConfigHandler::setShowStartupLaunchMessage(
  const bool showStartupLaunchMessage)
{
    setValue(QStringLiteral("showStartupLaunchMessage"),
             showStartupLaunchMessage);
}

int ConfigHandler::contrastOpacityValue()
{
    int val = value(QStringLiteral("contrastOpacity"), 190).toInt();
    return qBound(0, val, 255);
}

void ConfigHandler::setContrastOpacity(const int transparency)
{
    setValue(QStringLiteral("contrastOpacity"), transparency);
}

bool ConfigHandler::copyAndCloseAfterUploadEnabled()
{
    return value(QStringLiteral("copyAndCloseAfterUpload"), true).toBool();
}

void ConfigHandler::setCopyAndCloseAfterUploadEnabled(const bool value)
{
    setValue(QStringLiteral("copyAndCloseAfterUpload"), value);
}

bool ConfigHandler::historyConfirmationToDelete()
{
    return value(QStringLiteral("historyConfirmationToDelete"), true).toBool();
}

void ConfigHandler::setHistoryConfirmationToDelete(const bool check)
{
    setValue(QStringLiteral("historyConfirmationToDelete"), check);
}

int ConfigHandler::uploadHistoryMaxSizeValue()
{
    return value(QStringLiteral("uploadHistoryMax"), 25).toInt();
    ;
}

void ConfigHandler::setUploadHistoryMaxSize(const int max)
{
    setValue(QStringLiteral("uploadHistoryMax"), max);
}

bool ConfigHandler::saveAfterCopyValue()
{
    return value(QStringLiteral("saveAfterCopy")).toBool();
}

void ConfigHandler::setSaveAfterCopy(const bool save)
{
    setValue(QStringLiteral("saveAfterCopy"), save);
}

bool ConfigHandler::copyPathAfterSaveEnabled()
{
    return value(QStringLiteral("copyPathAfterSave"), false).toBool();
}

void ConfigHandler::setCopyPathAfterSaveEnabled(const bool value)
{
    setValue(QStringLiteral("copyPathAfterSave"), value);
}

bool ConfigHandler::useJpgForClipboard() const
{
#if !defined(Q_OS_MACOS)
    // FIXME - temporary fix to disable option for MacOS
    return value(QStringLiteral("useJpgForClipboard"), false).toBool();
#endif
}

void ConfigHandler::setUseJpgForClipboard(const bool value)
{
    setValue(QStringLiteral("useJpgForClipboard"), value);
}

void ConfigHandler::setSaveAsFileExtension(const QString& extension)
{
    setValue(QStringLiteral("setSaveAsFileExtension"), extension);
}

QString ConfigHandler::getSaveAsFileExtension()
{
    return m_settings
      .value(QStringLiteral("setSaveAsFileExtension"), QString(".png"))
      .toString();
}

void ConfigHandler::setDefaultSettings()
{
    foreach (const QString& key, m_settings.allKeys()) {
        if (key.startsWith("Shortcuts/")) {
            // Do not reset Shortcuts
            continue;
        }
        m_settings.remove(key);
    }
    m_settings.sync();
}

void ConfigHandler::setAllTheButtons()
{
    QVector<int> buttons =
      fromButtonToInt(CaptureToolButton::getIterableButtonTypes());
    // TODO: remove toList in v1.0
    setValue(QStringLiteral("buttons"), QVariant::fromValue(buttons.toList()));
}

QString ConfigHandler::configFilePath() const
{
    return m_settings.fileName();
}

bool ConfigHandler::normalizeButtons(QVector<int>& buttons)
{
    QVector<int> listTypesInt =
      fromButtonToInt(CaptureToolButton::getIterableButtonTypes());

    bool hasChanged = false;
    for (int i = 0; i < buttons.size(); i++) {
        if (!listTypesInt.contains(buttons.at(i))) {
            buttons.remove(i);
            hasChanged = true;
        }
    }
    return hasChanged;
}

QVector<CaptureToolButton::ButtonType> ConfigHandler::fromIntToButton(
  const QVector<int>& l)
{
    QVector<CaptureToolButton::ButtonType> buttons;
    for (auto const i : l)
        buttons << static_cast<CaptureToolButton::ButtonType>(i);
    return buttons;
}

QVector<int> ConfigHandler::fromButtonToInt(
  const QVector<CaptureToolButton::ButtonType>& l)
{
    QVector<int> buttons;
    for (auto const i : l)
        buttons << static_cast<int>(i);
    return buttons;
}

bool ConfigHandler::setShortcut(const QString& shortcutName,
                                const QString& shortutValue)
{
    bool error = false;
    m_settings.beginGroup("Shortcuts");

    QVector<QKeySequence> reservedShortcuts;

#if defined(Q_OS_MACOS)
    reservedShortcuts << QKeySequence(Qt::CTRL + Qt::Key_Backspace)
                      << QKeySequence(Qt::Key_Escape);
#else
    reservedShortcuts << QKeySequence(Qt::Key_Backspace)
                      << QKeySequence(Qt::Key_Escape);
#endif

    if (shortutValue.isEmpty()) {
        setValue(shortcutName, "");
    } else if (reservedShortcuts.contains(QKeySequence(shortutValue))) {
        // do not allow to set reserved shortcuts
        error = true;
    } else {
        // Make no difference for Return and Enter keys
        QString shortcutItem = shortutValue;
        if (shortcutItem == "Enter") {
            shortcutItem = QKeySequence(Qt::Key_Return).toString();
        }

        // do not allow to set overlapped shortcuts
        foreach (auto currentShortcutName, m_settings.allKeys()) {
            if (value(currentShortcutName) == shortcutItem) {
                setValue(shortcutName, "");
                error = true;
                break;
            }
        }
        if (!error) {
            setValue(shortcutName, shortcutItem);
        }
    }
    m_settings.endGroup();
    return !error;
}

const QString& ConfigHandler::shortcut(const QString& shortcutName)
{
    m_settings.beginGroup("Shortcuts");
    if (contains(shortcutName)) {
        m_strRes = value(shortcutName).toString();
    } else {
        m_strRes =
          ConfigShortcuts().captureShortcutDefault(shortcutName).toString();
    }
    m_settings.endGroup();
    return m_strRes;
}

void ConfigHandler::setValue(const QString& key, const QVariant& value)
{
    if (!hasError()) {
        m_settings.setValue(key, value);
    }
}

QVariant ConfigHandler::value(const QString& key,
                              const QVariant& fallback) const
{
    auto val = m_settings.value(key, fallback);
    if (hasError()) {
        return fallback;
    }
    return val;
}

/// Wrapper for QSettings::contains, but returns false if there is an error.
bool ConfigHandler::contains(const QString& key) const
{
    if (hasError()) {
        return false;
    }
    return m_settings.contains(key);
}

const QStringList& ConfigHandler::recognizedGeneralOptions() const
{
    static QStringList options = {
        // General tab in config window
        "showHelp",
        "showSidePanelButton",
        "showDesktopNotification",
        "disabledTrayIcon",
        "historyConfirmationToDelete",
        "checkForUpdates",
        "startupLaunch",
        "showStartupLaunchMessage",
        "copyAndCloseAfterUpload",
        "copyPathAfterSave",
        "useJpgForClipboard",
        "saveAfterCopy",
        "savePath",
        "savePathFixed",
        "uploadHistoryMax",
        "undoLimit",
        // Interface tab
        "uiColor",
        "contrastUiColor",
        "contrastOpacity",
        "buttons",
        // Filename Editor tab
        "filenamePattern",
        // Others
        "saveAfterCopyPath",
        "drawThickness",
        "drawColor",
        "userColors",
        "drawFontSize",
        "ignoreUpdateToVersion",
        "keepOpenAppLauncher",
    };
    return options;
}

QStringList ConfigHandler::recognizedShortcutNames() const
{
    // FIXME: Implement a more elegant solution in the future. Requires refactor
    QStringList names = {
        "TYPE_PENCIL",
        "TYPE_DRAWER",
        "TYPE_ARROW",
        "TYPE_SELECTION",
        "TYPE_RECTANGLE",
        "TYPE_CIRCLE",
        "TYPE_MARKER",
        "TYPE_MOVESELECTION",
        "TYPE_UNDO",
        "TYPE_COPY",
        "TYPE_SAVE",
        "TYPE_EXIT",
        "TYPE_IMAGEUPLOADER",
#if !defined(Q_OS_MACOS)
        "TYPE_OPEN_APP",
#endif
        "TYPE_PIXELATE",
        "TYPE_REDO",
        "TYPE_TEXT",
        "TYPE_TOGGLE_PANEL",
        "TYPE_RESIZE_LEFT",
        "TYPE_RESIZE_RIGHT",
        "TYPE_RESIZE_UP",
        "TYPE_RESIZE_DOWN",
        "TYPE_SELECT_ALL",
        "TYPE_MOVE_LEFT",
        "TYPE_MOVE_RIGHT",
        "TYPE_MOVE_UP",
        "TYPE_MOVE_DOWN",
        "TYPE_COMMIT_CURRENT_TOOL",
        "TYPE_DELETE_CURRENT_TOOL",
        "TYPE_PIN",
        "TYPE_SELECTIONINDICATOR",
        "TYPE_SIZEINCREASE",
        "TYPE_SIZEDECREASE",
        "TYPE_CIRCLECOUNT",
    };
    return names;
}

/// Return keys from group `group`. Use "General" for general settings.
QStringList ConfigHandler::keysFromGroup(const QString& group) const
{
    QStringList keys;
    for (const QString& key : m_settings.allKeys()) {
        if (group == "General" && !key.contains('/')) {
            keys.append(key);
        } else if (key.startsWith(group + "/")) {
            keys.append(key.mid(group.size() + 1));
        }
    }
    return keys;
}

bool ConfigHandler::isValidShortcutName(const QString& name) const
{
    // TODO
    return false;
}

void ConfigHandler::checkAndHandleError() const
{
    if (!QFile(m_settings.fileName()).exists()) {
        return;
    }
    if (!checkUnrecognizedSettings() || !checkShortcutConflicts()) {
        m_errorCheckPending = false;
        // do not spam the user with notifications
        if (!m_hasError) {
            // NOTE: m_hasError must be set before sending the notification
            // to avoid an infinite recursion caused by sendMessage calling
            // desktopNotificationValue()
            m_hasError = true;
            auto msg =
              "The configuration contains an error. Falling back to default.";
            SystemNotification().sendMessage(msg);
            emit error(msg);
        }
    } else {
        if (m_hasError) {
            // NOTE: m_hasError must be set before sending the notification.
            // Same reason as above.
            m_hasError = false;
            auto msg =
              "You have successfully resolved the configuration error.";
            SystemNotification().sendMessage(msg);
            emit errorResolved(msg);
        }
    }
    ensureFileWatched();
}

bool ConfigHandler::checkUnrecognizedSettings() const
{
    // sort the config keys by group
    QStringList generalKeys = keysFromGroup("General"),
                shortcutKeys = keysFromGroup("Shortcuts"),
                recognizedGeneralKeys = recognizedGeneralOptions(),
                recognizedShortcutKeys = recognizedShortcutNames();

    // form sets of unrecognized options by group
    QSet generalKeySet = QSet(generalKeys.begin(), generalKeys.end()),
         shortcutKeySet = QSet(shortcutKeys.begin(), shortcutKeys.end());
    generalKeySet.subtract(
      QSet(recognizedGeneralKeys.begin(), recognizedGeneralKeys.end()));
    shortcutKeySet.subtract(
      QSet(recognizedShortcutKeys.begin(), recognizedShortcutKeys.end()));

    // check if the sets are empty
    if (!generalKeySet.isEmpty() || !shortcutKeySet.isEmpty()) {
        return false; // error
    }
    return true; // ok
}

/// Check if there are multiple shortcuts with the same key binding.
bool ConfigHandler::checkShortcutConflicts() const
{
    bool ok = true;
    m_settings.beginGroup("Shortcuts");
    QStringList shortcuts = m_settings.allKeys();
    for (auto key1 = shortcuts.begin(); key1 != shortcuts.end(); ++key1) {
        for (auto key2 = key1 + 1; key2 != shortcuts.end(); ++key2) {
            if (m_settings.value(*key1).isNull() &&
                m_settings.value(*key1) == m_settings.value(*key2)) {
                ok = false;
                break;
            }
        }
    }
    m_settings.endGroup();
    return ok;
}

bool ConfigHandler::hasError() const
{
    if (m_errorCheckPending) {
        checkAndHandleError();
        m_errorCheckPending = false;
    }
    return m_hasError;
}

void ConfigHandler::ensureFileWatched() const
{
    QFile file(m_settings.fileName());
    if (!file.exists()) {
        file.open(QFileDevice::WriteOnly);
        file.close();
    }
    if (m_configWatcher != nullptr && m_configWatcher->files().isEmpty() &&
        qApp != nullptr // ensures that the organization name can be accessed
    ) {
        m_configWatcher->addPath(m_settings.fileName());
    }
}
