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
#include <QVector>
#include <algorithm>
#if defined(Q_OS_MACOS)
#include <QProcess>
#endif

// HELPER FUNCTIONS

QList<int> fromButtonToInt(const QList<CaptureToolButton::ButtonType>& l)
{
    QList<int> buttons;
    for (auto const i : l)
        buttons << static_cast<int>(i);
    return buttons;
}

QList<CaptureToolButton::ButtonType> fromIntToButton(const QList<int>& l)
{
    QList<CaptureToolButton::ButtonType> buttons;
    for (auto const i : l)
        buttons << static_cast<CaptureToolButton::ButtonType>(i);
    return buttons;
}

bool normalizeButtons(QList<int>& buttons)
{
    QList<int> listTypesInt =
      fromButtonToInt(CaptureToolButton::getIterableButtonTypes());

    bool hasChanged = false;
    for (int i = 0; i < buttons.size(); i++) {
        if (!listTypesInt.contains(buttons.at(i))) {
            buttons.removeAt(i);
            hasChanged = true;
        }
    }
    return hasChanged;
}

bool verifyLaunchFile()
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

// VALUE HANDLING

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
 * value has some custom constraints on it. TODO
 *
 * @note You will only need to override `get` if you have to change the value
 * that was read from the config file. If you are fine with the value as long as
 * it is error-free, you don't have to override it.
 *
 * @note Keep in mind that you will probably want `check` to return `true` for
 * invalid QVariant's (option not found in config file).
 *
 */
class ValueHandler
{
public:
    virtual bool check(const QVariant& val) = 0;
    virtual QVariant value(const QVariant& val)
    {
        if (!val.isValid() || !check(val)) {
            return fallback();
        } else {
            return process(val);
        }
    }
    virtual QVariant fallback() { return QVariant(); };
    virtual QVariant representation(const QVariant& val)
    {
        return val.toString();
    }

protected:
    virtual QVariant process(const QVariant& val) { return val; }
};

// CUSTOM TYPE HANDLERS

class Bool : public ValueHandler
{
public:
    Bool(bool def)
      : m_def(def)
    {}
    bool check(const QVariant& val) override
    {
        QString str = val.toString();
        if (str != "true" && str != "false") {
            return false;
        }
        return true;
    }
    QVariant fallback() override { return m_def; }

private:
    bool m_def;
};

class String : public ValueHandler
{
public:
    String(const QString& def)
      : m_def(def)
    {}
    bool check(const QVariant&) override { return true; }
    QVariant fallback() override { return m_def; }

private:
    QString m_def;
};

class Color : public ValueHandler
{
public:
    Color(const QColor& def)
      : m_def(def)
    {}
    bool check(const QVariant& val) override
    {
        return QColor::isValidColor(val.toString());
    }
    QVariant fallback() override { return m_def; }
    QVariant representation(const QVariant& val) override
    {
        return QString(val.value<QColor>().name());
    }

private:
    QColor m_def;
};

class BoundedInt : public ValueHandler
{
public:
    BoundedInt(int min, int max, int def)
      : m_min(min)
      , m_max(max)
      , m_def(def)
    {}

    bool check(const QVariant& val) override
    {
        QString str = val.toString();
        bool conversionOk;
        int num = str.toInt(&conversionOk);
        return conversionOk && (m_max < m_min || num <= m_max);
    }
    virtual QVariant fallback() override { return m_def; };

private:
    int m_min, m_max, m_def;
};

class LowerBoundedInt : public ValueHandler
{
public:
    LowerBoundedInt(int min, int def)
      : m_min(min)
      , m_def(def)
    {}
    bool check(const QVariant& val) override
    {
        QString str = val.toString();
        bool conversionOk;
        int num = str.toInt(&conversionOk);
        return conversionOk && num >= m_min;
    };
    virtual QVariant fallback() override { return m_def; }

private:
    int m_min, m_def;
};

class KeySequence : public ValueHandler
{
public:
    KeySequence(const QString& shortcutName)
      : m_shortcutName(shortcutName)
    {}
    bool check(const QVariant& val) override
    {
        // TODO
        return true;
    }
    QVariant fallback() override
    {
        return ConfigShortcuts()
          .captureShortcutDefault(m_shortcutName)
          .toString();
    }

private:
    QString m_shortcutName;
};

class ExistingDir : public ValueHandler
{
    bool check(const QVariant& val) override
    {
        if (!val.canConvert(QVariant::String) || val.toString().isEmpty()) {
            return false;
        }
        QFileInfo info(val.toString());
        return info.isDir() && info.exists();
    }

    QVariant fallback() override
    {
        QString path =
          QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
        if (!QFileInfo(path).isDir()) {
            // TODO can we rely on the fact that home exists?
            path =
              QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
        }
        return path;
    }
};

class FilenamePattern : public ValueHandler
{
    bool check(const QVariant&) override { return true; }
    QVariant fallback() override
    {
        return ConfigHandler().filenamePatternDefault();
    }
};

class ButtonList : public ValueHandler
{
    using BList = QList<CaptureToolButton::ButtonType>;
    bool check(const QVariant& val) override
    {
        auto allButtons = CaptureToolButton::getIterableButtonTypes();
        using CTB = CaptureToolButton;
        for (int btn : val.value<QList<int>>()) {
            if (!allButtons.contains(static_cast<CTB::ButtonType>(btn))) {
                return false;
            }
        }
        return true;
    }
    QVariant value(const QVariant& val) override
    {
        // Get unsorted button list
        BList buttons = ValueHandler::value(val).value<BList>();

        using BT = CaptureToolButton::ButtonType;
        std::sort(buttons.begin(), buttons.end(), [](BT a, BT b) {
            return CaptureToolButton::getPriorityByButton(a) <
                   CaptureToolButton::getPriorityByButton(b);
        });
        return QVariant::fromValue(buttons);
    }
    QVariant process(const QVariant& val) override
    {
        QList<int> intButtons = val.value<QList<int>>();
        auto buttons = fromIntToButton(intButtons);
        return QVariant::fromValue(buttons);
    }
    QVariant fallback() override
    {
        auto buttons = CaptureToolButton::getIterableButtonTypes();
        buttons.removeOne(CaptureToolButton::TYPE_SIZEDECREASE);
        buttons.removeOne(CaptureToolButton::TYPE_SIZEINCREASE);
        // TODO: remove toList in v1.0
        return QVariant::fromValue(buttons);
    }
    QVariant representation(const QVariant& val) override
    {
        auto intList = fromButtonToInt(val.value<BList>());
        normalizeButtons(intList);
        return QVariant::fromValue(intList);
    }
};

class UserColors : public ValueHandler
{
    bool check(const QVariant& val) override
    {
        if (!val.isValid()) {
            return true;
        }
        if (!val.canConvert(QVariant::StringList)) {
            return false;
        }
        for (const QString& str : val.toStringList()) {
            if (!QColor::isValidColor(str)) {
                return false;
            }
        }
        return true;
    }
    QVariant process(const QVariant& val) override
    {
        QStringList strColors = val.toStringList();
        if (strColors.isEmpty()) {
            return fallback();
        }

        QVector<QColor> colors;
        colors.reserve(strColors.size());

        for (const QString& str : strColors) {
            colors.append(QColor(str));
        }

        return QVariant::fromValue(colors);
    }
    QVariant fallback() override
    {
        return QVariant::fromValue(QVector<QColor>{ Qt::darkRed,
                                                    Qt::red,
                                                    Qt::yellow,
                                                    Qt::green,
                                                    Qt::darkGreen,
                                                    Qt::cyan,
                                                    Qt::blue,
                                                    Qt::magenta,
                                                    Qt::darkMagenta,
                                                    QColor() });
    }
};

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
// NOTE: Please keep it well structured
static QMap<class QString, QSharedPointer<ValueHandler>>
  recognizedGeneralOptions = {
      // clang-format off
    CUSTOM("showHelp"                    ,Bool               ( true          )),
    CUSTOM("showSidePanelButton"         ,Bool               ( true          )),
    CUSTOM("showDesktopNotification"     ,Bool               ( true          )),
    CUSTOM("disabledTrayIcon"            ,Bool               ( false         )),
    CUSTOM("historyConfirmationToDelete" ,Bool               ( true          )),
    CUSTOM("checkForUpdates"             ,Bool               ( true          )),
#if defined(Q_OS_MACOS)
    CUSTOM("startupLaunch"               ,Bool               ( false         )),
#else
    CUSTOM("startupLaunch"               ,Bool               ( true          )),
#endif
    CUSTOM("showStartupLaunchMessage"    ,Bool               ( true          )),
    CUSTOM("copyAndCloseAfterUpload"     ,Bool               ( true          )),
    CUSTOM("copyPathAfterSave"           ,Bool               ( false         )),
#if !defined(Q_OS_MACOS) // TODO is this the right way?
    CUSTOM("useJpgForClipboard"          ,Bool               ( false         )),
#endif
    // TODO obsolete?
    CUSTOM("saveAfterCopy"               ,Bool               ( false         )),
    CUSTOM("savePath"                    ,ExistingDir        (               )),
    CUSTOM("savePathFixed"               ,Bool               ( false         )),
    CUSTOM("uploadHistoryMax"            ,LowerBoundedInt(1  , 25            )),
    CUSTOM("undoLimit"                   ,BoundedInt(1, 999  , 100           )),
    // Interface tab
    CUSTOM("uiColor"                     ,Color              ( {116, 0, 150} )),
    CUSTOM("contrastUiColor"             ,Color              ( {39, 0, 50}   )),
    CUSTOM("contrastOpacity"             ,BoundedInt(0, 255  , 190           )),
    CUSTOM("buttons"                     ,ButtonList         ( {}            )),
    // Filename Editor tab
    CUSTOM("filenamePattern"             ,String             ( {}            )),
    // Others
    // TODO obsolete?
    CUSTOM("saveAfterCopyPath"           ,ExistingDir        (               )),
    CUSTOM("drawThickness"               ,LowerBoundedInt(1  , 3             )),
    CUSTOM("drawColor"                   ,Color              ( Qt::red       )),
    CUSTOM("userColors"                  ,UserColors         (               )),
    CUSTOM("drawFontSize"                ,LowerBoundedInt(1  , 8             )),
    CUSTOM("ignoreUpdateToVersion"       ,String             ( ""            )),
    CUSTOM("keepOpenAppLauncher"         ,Bool               ( false         )),
    CUSTOM("fontFamily"                  ,String             ( ""            )),
    CUSTOM("setSaveAsFileExtension"      ,String             ( ""            )),
      // clang-format on
  };

// CLASS CONFIGHANDLER

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

// SPECIAL CASES

bool ConfigHandler::startupLaunch()
{
    bool res = value(QStringLiteral("startupLaunch")).toBool();
    if (res != verifyLaunchFile()) {
        setStartupLaunch(res);
    }
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

QString ConfigHandler::saveAsFileExtension()
{
    // TODO If the name of the option changes in the future, remove this
    // function and use the macro CONFIG_GETTER_SETTER instead.
    return value("setSaveAsFileExtension").toString();
}

void ConfigHandler::setAllTheButtons()
{
    QList<int> buttons =
      fromButtonToInt(CaptureToolButton::getIterableButtonTypes());
    setValue(QStringLiteral("buttons"), QVariant::fromValue(buttons));
}

// DEFAULTS

QString ConfigHandler::filenamePatternDefault()
{
    return QStringLiteral("%F_%H-%M");
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

QString ConfigHandler::configFilePath() const
{
    return m_settings.fileName();
}

// GENERIC GETTERS AND SETTERS

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

QString ConfigHandler::shortcut(const QString& shortcutName)
{
    return value(QStringLiteral("Shortcuts/") + shortcutName).toString();
}

void ConfigHandler::setValue(const QString& key, const QVariant& value)
{
    assertKeyRecognized(key);
    if (!hasError()) {
        m_skipNextErrorCheck = true;
        auto val = valueHandler(key)->representation(value);
        m_settings.setValue(key, val);
    }
}

QVariant ConfigHandler::value(const QString& key) const
{
    assertKeyRecognized(key);
    // Perform check on entire config if due. Please make sure that this
    // function is called in all scenarios - best to keep it on top.
    hasError();

    auto val = m_settings.value(key);

    auto handler = valueHandler(key);

    // Check the value for semantic errors
    if (val.isValid() && !handler->check(val)) {
        setErrorState(true);
    }
    if (m_hasError) {
        return handler->fallback();
    }

    return handler->value(val);
}

const QSet<QString>& ConfigHandler::recognizedGeneralOptions() const
{
    static QSet<QString> options = QSet(::recognizedGeneralOptions.keyBegin(),
                                        ::recognizedGeneralOptions.keyEnd());
    return options;
}

const QSet<QString>& ConfigHandler::recognizedShortcutNames() const
{
    // FIXME: Implement a more elegant solution in the future. Requires refactor
    // in other classes
    static QSet<QString> names = {
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
QSet<QString> ConfigHandler::keysFromGroup(const QString& group) const
{
    QSet<QString> keys;
    for (const QString& key : m_settings.allKeys()) {
        if (group == "General" && !key.contains('/')) {
            keys.insert(key);
        } else if (key.startsWith(group + "/")) {
            keys.insert(key.mid(group.size() + 1));
        }
    }
    return keys;
}

// ERROR HANDLING

void ConfigHandler::checkAndHandleError() const
{
    if (!QFile(m_settings.fileName()).exists()) {
        setErrorState(false);
    } else {
        setErrorState(!checkUnrecognizedSettings() ||
                      !checkShortcutConflicts() || !checkSemantics());
    }

    ensureFileWatched();
}

bool ConfigHandler::checkUnrecognizedSettings() const
{
    // sort the config keys by group
    QSet<QString> generalKeys = keysFromGroup("General"),
                  shortcutKeys = keysFromGroup("Shortcuts"),
                  recognizedGeneralKeys = recognizedGeneralOptions(),
                  recognizedShortcutKeys = recognizedShortcutNames();

    // subtract recognized keys
    generalKeys.subtract(recognizedGeneralKeys);
    shortcutKeys.subtract(recognizedShortcutKeys);

    // what is left are the unrecognized keys - hopefully empty
    if (!generalKeys.isEmpty() || !shortcutKeys.isEmpty()) {
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

bool ConfigHandler::checkSemantics() const
{
    QStringList allKeys = m_settings.allKeys();
    for (const QString& key : allKeys) {
        QVariant val = m_settings.value(key);
        // obtain a handler for the value
        if (val.isValid() && !valueHandler(key)->check(val)) {
            return false;
        }
    }
    return true;
}

void ConfigHandler::setErrorState(bool error) const
{
    bool hadError = m_hasError;
    m_hasError = error;
    // Notify user every time m_hasError changes
    if (!hadError && m_hasError) {
        QString msg = errorMessage();
        SystemNotification().sendMessage(msg);
        emit getInstance()->error();
    } else if (hadError && !m_hasError) {
        auto msg =
          tr("You have successfully resolved the configuration error.");
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
    return tr("The configuration contains an error. Falling back to default.");
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

QSharedPointer<ValueHandler> ConfigHandler::valueHandler(
  const QString& key) const
{
    QSharedPointer<ValueHandler> handler;
    if (m_settings.group() == QStringLiteral("Shortcuts") ||
        key.startsWith("Shortcuts/")) {
        QString _key = key;
        _key.replace("Shortcuts/", "");
        handler.reset(new KeySequence(_key));
    } else { // General group
        handler = ::recognizedGeneralOptions.value(key);
    }
    return handler;
}

void ConfigHandler::assertKeyRecognized(const QString& key) const
{
    bool recognized = key.startsWith(QStringLiteral("Shortcuts/"))
                        ? recognizedShortcutNames().contains(key.mid(10))
                        : ::recognizedGeneralOptions.contains(key);
    if (!recognized) {
#if defined(QT_DEBUG)
        // This should never happen, but just in case
        throw std::logic_error(
          tr("Bad config key '%1' in ConfigHandler. Please report "
             "this as a bug.")
            .arg(key)
            .toStdString());
#else
        setNewErrorState(true);
#endif
    }
}

// STATIC MEMBER DEFINITIONS

bool ConfigHandler::m_hasError = false;
bool ConfigHandler::m_errorCheckPending = false;
bool ConfigHandler::m_skipNextErrorCheck = false;
QSharedPointer<QFileSystemWatcher> ConfigHandler::m_configWatcher;
