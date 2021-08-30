// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "confighandler.h"
#include "src/tools/capturetool.h"
#include "src/utils/configshortcuts.h"
#include "systemnotification.h"
#include "valuehandler.h"
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
 * Use this to declare a setting with a type that is either unrecognized by
 * QVariant or if you need to place additional constraints on its value.
 * @param KEY Name of the setting as in the config file
 *            (a c-style string literal)
 * @param TYPE An instance of a `ValueHandler` derivative. This must be
 * specified in the form of a constructor, or the macro will misbehave.
 */
#define OPTION(KEY, TYPE)                                                      \
    {                                                                          \
        QStringLiteral(KEY), QSharedPointer<ValueHandler>(new TYPE)            \
    }

// This map contains all the information that is needed to parse, verify and
// preprocess each configuration option in the General section.
// NOTE: Please keep it well structured
static QMap<class QString, QSharedPointer<ValueHandler>>
  recognizedGeneralOptions = {
      // clang-format off
    OPTION("showHelp"                    ,Bool               ( true          )),
    OPTION("showSidePanelButton"         ,Bool               ( true          )),
    OPTION("showDesktopNotification"     ,Bool               ( true          )),
    OPTION("disabledTrayIcon"            ,Bool               ( false         )),
    OPTION("historyConfirmationToDelete" ,Bool               ( true          )),
    OPTION("checkForUpdates"             ,Bool               ( true          )),
#if defined(Q_OS_MACOS)
    CUSTOM("startupLaunch"               ,Bool               ( false         )),
#else
    OPTION("startupLaunch"               ,Bool               ( true          )),
#endif
    OPTION("showStartupLaunchMessage"    ,Bool               ( true          )),
    OPTION("copyAndCloseAfterUpload"     ,Bool               ( true          )),
    OPTION("copyPathAfterSave"           ,Bool               ( false         )),
#if !defined(Q_OS_MACOS) // TODO is this the right way?
    OPTION("useJpgForClipboard"          ,Bool               ( false         )),
#endif
    // TODO obsolete?
    OPTION("saveAfterCopy"               ,Bool               ( false         )),
    OPTION("savePath"                    ,ExistingDir        (               )),
    OPTION("savePathFixed"               ,Bool               ( false         )),
    OPTION("uploadHistoryMax"            ,LowerBoundedInt(1  , 25            )),
    OPTION("undoLimit"                   ,BoundedInt(1, 999  , 100           )),
    // Interface tab
    OPTION("uiColor"                     ,Color              ( {116, 0, 150} )),
    OPTION("contrastUiColor"             ,Color              ( {39, 0, 50}   )),
    OPTION("contrastOpacity"             ,BoundedInt(0, 255  , 190           )),
    OPTION("buttons"                     ,ButtonList         ( {}            )),
    // Filename Editor tab
    OPTION("filenamePattern"             ,String             ( {}            )),
    // Others
    // TODO obsolete?
    OPTION("saveAfterCopyPath"           ,ExistingDir        (               )),
    OPTION("drawThickness"               ,LowerBoundedInt(1  , 3             )),
    OPTION("drawColor"                   ,Color              ( Qt::red       )),
    OPTION("userColors"                  ,UserColors         (               )),
    OPTION("drawFontSize"                ,LowerBoundedInt(1  , 8             )),
    OPTION("ignoreUpdateToVersion"       ,String             ( ""            )),
    OPTION("keepOpenAppLauncher"         ,Bool               ( false         )),
    OPTION("fontFamily"                  ,String             ( ""            )),
    OPTION("setSaveAsFileExtension"      ,String             ( ""            )),
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
      ButtonList::toIntList(CaptureToolButton::getIterableButtonTypes());
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
            if (m_settings.value(*key1).isValid() &&
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
