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
#include <QKeySequence>
#include <QStandardPaths>
#include <algorithm>
#if defined(Q_OS_MACOS)
#include <QProcess>
#endif

ConfigHandler::ConfigHandler()
{
    m_settings.setDefaultFormat(QSettings::IniFormat);
}

QVector<CaptureToolButton::ButtonType> ConfigHandler::getButtons()
{
    QVector<CaptureToolButton::ButtonType> buttons;
    if (m_settings.contains(QStringLiteral("buttons"))) {
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

    if (m_settings.contains(QStringLiteral("userColors"))) {
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
    if (!m_settings.contains(QStringLiteral("savePathFixed"))) {
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

    if (m_settings.contains(QStringLiteral("uiColor"))) {
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

    if (m_settings.contains(QStringLiteral("contrastUiColor"))) {
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

    if (m_settings.contains(QStringLiteral("drawColor"))) {
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
    m_strRes.clear();
    if (m_settings.contains(QStringLiteral("fontFamily"))) {
        m_strRes = value(QStringLiteral("fontFamily")).toString();
    }
    return m_strRes;
}

bool ConfigHandler::showHelpValue()
{
    bool res = true;
    if (m_settings.contains(QStringLiteral("showHelp"))) {
        res = value(QStringLiteral("showHelp")).toBool();
    }
    return res;
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
    int limit = 100;
    if (m_settings.contains(QStringLiteral("undoLimit"))) {
        limit = value(QStringLiteral("undoLimit")).toInt();
        limit = qBound(1, limit, 999);
    }
    return limit;
}

bool ConfigHandler::desktopNotificationValue()
{
    bool res = true;
    if (m_settings.contains(QStringLiteral("showDesktopNotification"))) {
        res = value(QStringLiteral("showDesktopNotification")).toBool();
    }
    return res;
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
    bool res = false;
    if (m_settings.contains(QStringLiteral("disabledTrayIcon"))) {
        res = value(QStringLiteral("disabledTrayIcon")).toBool();
    }
    return res;
}

void ConfigHandler::setDisabledTrayIcon(const bool disabledTrayIcon)
{
    setValue(QStringLiteral("disabledTrayIcon"), disabledTrayIcon);
}

int ConfigHandler::drawThicknessValue()
{
    int res = 3;
    if (m_settings.contains(QStringLiteral("drawThickness"))) {
        res = value(QStringLiteral("drawThickness")).toInt();
    }
    return res;
}

void ConfigHandler::setDrawThickness(const int thickness)
{
    setValue(QStringLiteral("drawThickness"), thickness);
}

int ConfigHandler::drawFontSizeValue()
{
    int res = 8;
    if (m_settings.contains(QStringLiteral("drawFontSize"))) {
        res = value(QStringLiteral("drawFontSize")).toInt();
    }
    return res;
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
    bool res = true;
    if (m_settings.contains(QStringLiteral("checkForUpdates"))) {
        res = value(QStringLiteral("checkForUpdates")).toBool();
    }
    return res;
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
    if (m_settings.contains(QStringLiteral("startupLaunch"))) {
        res = value(QStringLiteral("startupLaunch")).toBool();
    }
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
    if (!m_settings.contains(QStringLiteral("showStartupLaunchMessage"))) {
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
    int opacity = 190;
    if (m_settings.contains(QStringLiteral("contrastOpacity"))) {
        opacity = value(QStringLiteral("contrastOpacity")).toInt();
        opacity = qBound(0, opacity, 255);
    }
    return opacity;
}

void ConfigHandler::setContrastOpacity(const int transparency)
{
    setValue(QStringLiteral("contrastOpacity"), transparency);
}

bool ConfigHandler::copyAndCloseAfterUploadEnabled()
{
    bool res = true;
    if (m_settings.contains(QStringLiteral("copyAndCloseAfterUpload"))) {
        res = value(QStringLiteral("copyAndCloseAfterUpload")).toBool();
    }
    return res;
}

void ConfigHandler::setCopyAndCloseAfterUploadEnabled(const bool value)
{
    setValue(QStringLiteral("copyAndCloseAfterUpload"), value);
}

bool ConfigHandler::historyConfirmationToDelete()
{
    bool res = true;
    if (m_settings.contains(QStringLiteral("historyConfirmationToDelete"))) {
        res = value(QStringLiteral("historyConfirmationToDelete")).toBool();
    }
    return res;
}

void ConfigHandler::setHistoryConfirmationToDelete(const bool check)
{
    setValue(QStringLiteral("historyConfirmationToDelete"), check);
}

int ConfigHandler::uploadHistoryMaxSizeValue()
{
    int max = 25;
    if (m_settings.contains(QStringLiteral("uploadHistoryMax"))) {
        max = value(QStringLiteral("uploadHistoryMax")).toInt();
    }
    return max;
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
    bool res = false;
    if (m_settings.contains(QStringLiteral("copyPathAfterSave"))) {
        res = value(QStringLiteral("copyPathAfterSave")).toBool();
    }
    return res;
}

void ConfigHandler::setCopyPathAfterSaveEnabled(const bool value)
{
    setValue(QStringLiteral("copyPathAfterSave"), value);
}

bool ConfigHandler::useJpgForClipboard() const
{
#if !defined(Q_OS_MACOS)
    // FIXME - temporary fix to disable option for MacOS
    if (m_settings.contains(QStringLiteral("useJpgForClipboard"))) {
        return value(QStringLiteral("useJpgForClipboard")).toBool();
    }
#endif
    return false;
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
    if (m_settings.contains(shortcutName)) {
        m_strRes = m_settings.value(shortcutName).toString();
    } else {
        m_strRes =
          ConfigShortcuts().captureShortcutDefault(shortcutName).toString();
    }
    m_settings.endGroup();
    return m_strRes;
}

void ConfigHandler::setValue(const QString& key, const QVariant& value)
{
    m_settings.setValue(key, value);
    auto status = m_settings.status();
    checkAndHandleError();
}

QVariant ConfigHandler::value(const QString& key,
                              const QVariant& fallback) const
{
    auto val = m_settings.value(key, fallback);
    if (!checkAndHandleError()) {
        return fallback;
    }
    return val;
}

const QStringList& ConfigHandler::recognizedGeneralOptions() const
{
    static QStringList options = { // General tab in config window
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
                                   "drawColor"
    };
    return options;
}

QStringList ConfigHandler::recognizedShortcutNames() const
{
    // TODO
    return {};
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

bool ConfigHandler::checkAndHandleError() const
{
    if (!checkUnrecognizedSettings()) {
        auto msg =
          "The configuration contains an error. Falling back to default.";
        SystemNotification().sendMessage(msg);
        emit error(msg);
    }
    return false;
}

bool ConfigHandler::checkUnrecognizedSettings() const
{
    // sort the keys by group
    QStringList generalKeys = keysFromGroup("General"),
                shortcutKeys = keysFromGroup("Shortcuts"),
                recognizedGeneralKeys = recognizedGeneralOptions();

    auto generalKeySet = QSet(generalKeys.begin(), generalKeys.end());
    generalKeySet.subtract(
      QSet(recognizedGeneralKeys.begin(), recognizedGeneralKeys.end()));

    // check for outliers in [General]
    if (!generalKeySet.isEmpty()) {
        return false;
    }
    return true;
}
