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
#include <QMap>
#include <QSharedPointer>
#include <QStandardPaths>
#include <algorithm>
#if defined(Q_OS_MACOS)
#include <QProcess>
#endif

// HELPER FUNCTIONS

QVector<int> fromButtonToInt(const QVector<CaptureToolButton::ButtonType>& l)
{
    QVector<int> buttons;
    for (auto const i : l)
        buttons << static_cast<int>(i);
    return buttons;
}

QVector<CaptureToolButton::ButtonType> fromIntToButton(const QVector<int>& l)
{
    QVector<CaptureToolButton::ButtonType> buttons;
    for (auto const i : l)
        buttons << static_cast<CaptureToolButton::ButtonType>(i);
    return buttons;
}

bool normalizeButtons(QVector<int>& buttons)
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

/**
 * Handles the value of a configuration option.
 *
 * Each configuration option should usually be handled in three different ways:
 * - have its value checked for errors (type, format, etc.)
 * - have its value (that was taken from the config file) adapted for proper use
 * - provided a fallback value in case: the config does not explicitly specify
 *   it, or the config contains an error and is globally falling back to
 *   defaults
 *
 * Subclass this class to handle custom value types.
 *
 * If you wish to handle simple value types (those supported by QVariant) you
 * should use `SimpleValueHandler`. Note that you can't use that class if the
 * value has some custom constraints on it.
 *
 * @note You will only need to override `get` if you have to change the value
 * that was read from the config file. If you are fine with the value as long as
 * it is error-free, you don't have to override it.
 *
 */
class ValueHandler
{
public:
    virtual bool check(const QVariant& val) = 0;
    virtual QVariant get(const QVariant& val)
    {
        return val.isValid() ? val : fallback();
    }
    virtual QVariant fallback() { return QVariant(); };
};

class SimpleValueHandler : public ValueHandler
{
public:
    SimpleValueHandler(QVariant::Type type = QVariant::UserType,
                       const QVariant& def = {})
      : m_defaultValue(def)
      , m_type(type)
    {}
    bool check(const QVariant& val) override
    {
        // An invalid value means that the setting is not found in the config
        return !val.isValid() || val.canConvert(m_type);
    }

    virtual QVariant fallback() override { return m_defaultValue; };

protected:
    QVariant m_defaultValue;
    QVariant::Type m_type;
};

// CUSTOM TYPE HANDLERS

class BoundedInt : public ValueHandler
{
public:
    BoundedInt(int min, int max, int def)
      : m_min(min)
      , m_max(max)
      , m_def(def)
    {}
    BoundedInt(int min, int def)
      : m_min(min)
      , m_max(min - 1)
      , m_def(def)
    {}

    bool check(const QVariant& val) override
    {
        if (!val.canConvert(QVariant::Int)) {
            return false;
        }
        int num = val.toInt();
        return m_min <= num && (m_max < m_min || num <= m_max);
    }
    virtual QVariant fallback() override { return m_def; };

private:
    int m_min, m_max, m_def;
};

class ExistingDir : public SimpleValueHandler
{
    bool check(const QVariant& val) override
    {
        QSharedPointer<int>(new int);
        if (!val.canConvert(QVariant::String) || val.toString().isEmpty()) {
            return false;
        }
        QFileInfo info(val.toString());
        return info.isDir() && info.exists();
    }
};

class FilenamePattern : public ValueHandler
{
    bool check(const QVariant&) override { return true; }
    QVariant get(const QVariant& val) override
    {
        return !val.isNull() ? val : fallback();
    }
    QVariant fallback() override
    {
        return ConfigHandler().filenamePatternDefault();
    }
};

class Buttons : public SimpleValueHandler
{
public:
    bool check(const QVariant& val) override
    {
        if (!val.canConvert<QList<int>>()) {
            return false;
        }
        auto allButtons = CaptureToolButton::getIterableButtonTypes();
        using CTB = CaptureToolButton;
        for (int btn : val.value<QList<int>>()) {
            if (!allButtons.contains(static_cast<CTB::ButtonType>(btn))) {
                return false;
            }
        }
        return true;
    }
    QVariant fallback() override
    {
        auto buttons = CaptureToolButton::getIterableButtonTypes();
        buttons.removeOne(CaptureToolButton::TYPE_SIZEDECREASE);
        buttons.removeOne(CaptureToolButton::TYPE_SIZEINCREASE);
        // TODO: remove toList in v1.0
        return QVariant::fromValue(fromButtonToInt(buttons).toList());
    }
};

class UserColors : public SimpleValueHandler
{
    bool check(const QVariant& val) override
    {
        return false; // TODO
    }
};

using SVH = SimpleValueHandler;
/**
 * Use this to declare a setting with a type that is recognized by QVariant.
 *
 * @param KEY Name of the setting as in the config file
 *            (a c-style string literal)
 * @param TYPE One of QVariant::Type. You only need to specify the name of the
 *             type, the macro will prefix it with QVariant::
 * @param DEF Default value
 */
#define SIMPLE(KEY, TYPE, DEF)                                                 \
    {                                                                          \
        QStringLiteral(KEY), QSharedPointer<SVH>(new SVH(QVariant::TYPE, DEF)) \
    }
/**
 * Use this to declare a setting with a type that is either unrecognized by
 * QVariant or if you need to place additional constraints on its value.
 * @param KEY Name of the setting as in the config file
 *            (a c-style string literal)
 * @param TYPE An instance of a `ValueHandler` derivative. This must be
 * specified in the form of a constructor, or the macro will misbehave.
 */
#define CUSTOM(KEY, TYPE)                                                      \
    {                                                                          \
        QStringLiteral(KEY), QSharedPointer<ValueHandler>(new TYPE)            \
    }

// This map contains all the information that is needed to parse, verify and
// preprocess each configuration option in the General section.
// Please keep it well structured
// clang-format off
static QMap<class QString, QSharedPointer<ValueHandler>>
  recognizedGeneralOptions = {
    SIMPLE("showHelp"                    ,Bool              ,true               ),
    SIMPLE("showSidePanelButton"         ,Bool              ,true               ),
    SIMPLE("showDesktopNotification"     ,Bool              ,true               ),
    SIMPLE("disabledTrayIcon"            ,Bool              ,false              ),
    SIMPLE("historyConfirmationToDelete" ,Bool              ,true               ),
    SIMPLE("checkForUpdates"             ,Bool              ,true               ),
#if defined(Q_OS_MACOS)
    SIMPLE("startupLaunch"               ,Bool              ,false              ),
#else
    SIMPLE("startupLaunch"               ,Bool              ,true               ),
#endif
    SIMPLE("showStartupLaunchMessage"    ,Bool              ,true               ),
    SIMPLE("copyAndCloseAfterUpload"     ,Bool              ,true               ),
    SIMPLE("copyPathAfterSave"           ,Bool              ,false              ),
    SIMPLE("useJpgForClipboard"          ,Bool              ,false              ),
    // TODO obsolete?
    SIMPLE("saveAfterCopy"               ,Bool              ,false              ),
    CUSTOM("savePath"                    ,ExistingDir        {}                 ),
    SIMPLE("savePathFixed"               ,Bool              ,false              ),
    CUSTOM("uploadHistoryMax"            ,BoundedInt(1      ,25)                ),
    CUSTOM("undoLimit"                   ,BoundedInt(1, 999 ,100)               ),
    // Interface tab
    SIMPLE("uiColor"                     ,Color             ,QColor(116, 0, 150)),
    SIMPLE("contrastUiColor"             ,Color             ,QColor(39, 0, 50)  ),
    CUSTOM("contrastOpacity"             ,BoundedInt(0, 255 ,190)               ),
    CUSTOM("buttons"                     ,Buttons            {}                 ),
    // Filename Editor tab
    SIMPLE("filenamePattern"             ,String            ,{}                 ),
    // Others
    // TODO obsolete?
    CUSTOM("saveAfterCopyPath"           ,ExistingDir        {}                 ),
    SIMPLE("drawThickness"               ,UInt              ,3                  ),
    SIMPLE("drawColor"                   ,Color             ,QColor(Qt::red)    ),
    CUSTOM("userColors"                  ,UserColors         {}                 ),
    SIMPLE("drawFontSize"                ,UInt              ,8                  ),
    SIMPLE("ignoreUpdateToVersion"       ,String            ,""                 ),
    SIMPLE("keepOpenAppLauncher"         ,Bool              ,false              ),
    SIMPLE("fontFamily"         		 ,String            ,false              ),
};
// clang-format on

// class ConfigHandler

bool ConfigHandler::m_hasError = false;
bool ConfigHandler::m_errorCheckPending = false;
bool ConfigHandler::m_skipNextErrorCheck = false;
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
                             emit getInstance()->fileChanged();

                             if (QFile(fileName).exists()) {
                                 m_configWatcher->addPath(fileName);
                             }
                             if (m_skipNextErrorCheck) {
                                 m_skipNextErrorCheck = false;
                                 return;
                             }
                             ConfigHandler().checkAndHandleError();
                             if (!QFile(fileName).exists()) {
                                 // File watcher stops watching a deleted file.
                                 // Next time the config is accessed, force it
                                 // to check for errors (and watch again).
                                 m_errorCheckPending = true;
                             }
                         });
    }
}

/// Serves as an object to which slots can be connected.
ConfigHandler* ConfigHandler::getInstance()
{
    static ConfigHandler config;
    return &config;
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
    if (contains(shortcutName)) {
        m_strRes =
          value(QStringLiteral("Shortcuts/") + shortcutName).toString();
    } else {
        m_strRes =
          ConfigShortcuts().captureShortcutDefault(shortcutName).toString();
    }
    return m_strRes;
}

void ConfigHandler::setValue(const QString& key, const QVariant& value)
{
    if (!hasError()) {
        m_skipNextErrorCheck = true;
        m_settings.setValue(key, value);
    }
}

QVariant ConfigHandler::value(const QString& key,
                              const QVariant& fallback) const
{
    // TODO remove fallback argument

    // Perform check on entire config if due. Please make sure that this
    // function is called in all scenarios - best to keep it as the first
    // statement.
    hasError();

    auto val = m_settings.value(key, fallback);

    // Check the value for semantic errors
    if (m_settings.group() == QStringLiteral("Shortcuts") ||
        key.startsWith("Shortcuts/")) {
        if (!SimpleValueHandler(QVariant::KeySequence).check(val)) {
            handleNewErrorState(true);
        }
        if (m_hasError) {
            return QVariant();
        }
    } else { // general option
        ValueHandler* handler = ::recognizedGeneralOptions[key].get();
        if (!handler->check(val)) {
            handleNewErrorState(true);
        }
        if (m_hasError) {
            handler->fallback();
        }
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
    static QStringList options = ::recognizedGeneralOptions.keys();
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
        handleNewErrorState(false);
    } else {
        handleNewErrorState(!checkUnrecognizedSettings() ||
                            !checkShortcutConflicts());
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

void ConfigHandler::handleNewErrorState(bool error) const
{
    bool hadError = m_hasError;
    m_hasError = error;
    // Notify user every time m_hasError changes
    if (!hadError && m_hasError) {
        QString msg = errorMessage();
        SystemNotification().sendMessage(msg);
        emit getInstance()->error();
    } else if (hadError && !m_hasError) {
        auto msg = "You have successfully resolved the configuration error.";
        SystemNotification().sendMessage(msg);
        emit getInstance()->errorResolved();
    }
}

bool ConfigHandler::hasError() const
{
    if (m_errorCheckPending) {
        checkAndHandleError();
        m_errorCheckPending = false;
    }
    return m_hasError;
}

QString ConfigHandler::errorMessage() const
{
    return QStringLiteral(
      "The configuration contains an error. Falling back to default.");
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
