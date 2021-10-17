// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "confighandler.h"
#include "src/tools/capturetool.h"
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
#include <QTextStream>
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
 *            (a C-style string literal)
 * @param TYPE An instance of a `ValueHandler` derivative. This must be
 *             specified in the form of a constructor, or the macro will
 *             misbehave.
 */
#define OPTION(KEY, TYPE)                                                      \
    {                                                                          \
        QStringLiteral(KEY), QSharedPointer<ValueHandler>(new TYPE)            \
    }

#define SHORTCUT(NAME, DEFAULT_VALUE)                                          \
    {                                                                          \
        QStringLiteral(NAME), QSharedPointer<KeySequence>(new KeySequence(     \
                                QKeySequence(QLatin1String(DEFAULT_VALUE))))   \
    }

/**
 * This map contains all the information that is needed to parse, verify and
 * preprocess each configuration option in the General section.
 * NOTE: Please keep it well structured
 */
// clang-format off
static QMap<class QString, QSharedPointer<ValueHandler>>
  recognizedGeneralOptions = {
//         KEY                            TYPE                 DEFAULT_VALUE
    OPTION("showHelp"                    ,Bool               ( true          )),
    OPTION("showSidePanelButton"         ,Bool               ( true          )),
    OPTION("showDesktopNotification"     ,Bool               ( true          )),
    OPTION("disabledTrayIcon"            ,Bool               ( false         )),
    OPTION("historyConfirmationToDelete" ,Bool               ( true          )),
    OPTION("checkForUpdates"             ,Bool               ( true          )),
#if defined(Q_OS_MACOS)
    OPTION("startupLaunch"               ,Bool               ( false         )),
#else
    OPTION("startupLaunch"               ,Bool               ( true          )),
#endif
    OPTION("showStartupLaunchMessage"    ,Bool               ( true          )),
    OPTION("copyAndCloseAfterUpload"     ,Bool               ( true          )),
    OPTION("copyPathAfterSave"           ,Bool               ( false         )),
#if !defined(Q_OS_MACOS)
    OPTION("useJpgForClipboard"          ,Bool               ( false         )),
#endif
    OPTION("saveAfterCopy"               ,Bool               ( false         )),
    OPTION("savePath"                    ,ExistingDir        (               )),
    OPTION("savePathFixed"               ,Bool               ( false         )),
    OPTION("uploadHistoryMax"            ,LowerBoundedInt(0  , 25            )),
    OPTION("undoLimit"                   ,BoundedInt(0, 999  , 100           )),
    // Interface tab
    OPTION("uiColor"                     ,Color              ( {116, 0, 150} )),
    OPTION("contrastUiColor"             ,Color              ( {39, 0, 50}   )),
    OPTION("contrastOpacity"             ,BoundedInt(0, 255  , 190           )),
    OPTION("buttons"                     ,ButtonList         ( {}            )),
    // Filename Editor tab
    OPTION("filenamePattern"             ,FilenamePattern    ( {}            )),
    // Others
    OPTION("drawThickness"               ,LowerBoundedInt(1  , 3             )),
    OPTION("drawColor"                   ,Color              ( Qt::red       )),
    OPTION("userColors"                  ,UserColors         (               )),
    OPTION("drawFontSize"                ,LowerBoundedInt(1  , 8             )),
    OPTION("ignoreUpdateToVersion"       ,String             ( ""            )),
    OPTION("keepOpenAppLauncher"         ,Bool               ( false         )),
    OPTION("fontFamily"                  ,String             ( ""            )),
    OPTION("setSaveAsFileExtension"      ,String             ( ""            )),
  };

static QMap<QString, QSharedPointer<KeySequence>> recognizedShortcuts = {
//           NAME                           DEFAULT_SHORTCUT
    SHORTCUT("TYPE_PENCIL"              ,   "P"                     ),
    SHORTCUT("TYPE_DRAWER"              ,   "D"                     ),
    SHORTCUT("TYPE_ARROW"               ,   "A"                     ),
    SHORTCUT("TYPE_SELECTION"           ,   "S"                     ),
    SHORTCUT("TYPE_RECTANGLE"           ,   "R"                     ),
    SHORTCUT("TYPE_CIRCLE"              ,   "C"                     ),
    SHORTCUT("TYPE_MARKER"              ,   "M"                     ),
    SHORTCUT("TYPE_MOVESELECTION"       ,   "Ctrl+M"                ),
    SHORTCUT("TYPE_UNDO"                ,   "Ctrl+Z"                ),
    SHORTCUT("TYPE_COPY"                ,   "Ctrl+C"                ),
    SHORTCUT("TYPE_SAVE"                ,   "Ctrl+S"                ),
    SHORTCUT("TYPE_ACCEPT"              ,   "Return"                ),
    SHORTCUT("TYPE_EXIT"                ,   "Ctrl+Q"                ),
    SHORTCUT("TYPE_IMAGEUPLOADER"       ,   "Ctrl+U"                ),
#if !defined(Q_OS_MACOS)
    SHORTCUT("TYPE_OPEN_APP"            ,   "Ctrl+O"                ),
#endif
    SHORTCUT("TYPE_PIXELATE"            ,   "B"                     ),
    SHORTCUT("TYPE_INVERT"              ,   "I"                     ),
    SHORTCUT("TYPE_REDO"                ,   "Ctrl+Shift+Z"          ),
    SHORTCUT("TYPE_TEXT"                ,   "T"                     ),
    SHORTCUT("TYPE_TOGGLE_PANEL"        ,   "Space"                 ),
    SHORTCUT("TYPE_RESIZE_LEFT"         ,   "Shift+Left"            ),
    SHORTCUT("TYPE_RESIZE_RIGHT"        ,   "Shift+Right"           ),
    SHORTCUT("TYPE_RESIZE_UP"           ,   "Shift+Up"              ),
    SHORTCUT("TYPE_RESIZE_DOWN"         ,   "Shift+Down"            ),
    SHORTCUT("TYPE_SELECT_ALL"          ,   "Ctrl+A"                ),
    SHORTCUT("TYPE_MOVE_LEFT"           ,   "Left"                  ),
    SHORTCUT("TYPE_MOVE_RIGHT"          ,   "Right"                 ),
    SHORTCUT("TYPE_MOVE_UP"             ,   "Up"                    ),
    SHORTCUT("TYPE_MOVE_DOWN"           ,   "Down"                  ),
    SHORTCUT("TYPE_COMMIT_CURRENT_TOOL" ,   "Ctrl+Return"           ),
#if defined(Q_OS_MACOS)
    SHORTCUT("TYPE_DELETE_CURRENT_TOOL" ,   "Backspace"             ),
#else
    SHORTCUT("TYPE_DELETE_CURRENT_TOOL" ,   "Delete"                ),
#endif
    SHORTCUT("TYPE_PIN"                 ,                           ),
    SHORTCUT("TYPE_SELECTIONINDICATOR"  ,                           ),
    SHORTCUT("TYPE_SIZEINCREASE"        ,                           ),
    SHORTCUT("TYPE_SIZEDECREASE"        ,                           ),
    SHORTCUT("TYPE_CIRCLECOUNT"         ,                           ),
};
// clang-format on

// CLASS CONFIGHANDLER

ConfigHandler::ConfigHandler(bool skipInitialErrorCheck)
  : m_settings(QSettings::IniFormat,
               QSettings::UserScope,
               qApp->organizationName(),
               qApp->applicationName())
{

    static bool wasEverChecked = false;
    if (!skipInitialErrorCheck && !wasEverChecked) {
        // check for error on initial call
        checkAndHandleError();
        wasEverChecked = true;
    }
    if (m_configWatcher == nullptr) {
        // check for error every time the file changes
        m_configWatcher.reset(new QFileSystemWatcher());
        ensureFileWatched();
        QObject::connect(m_configWatcher.data(),
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
    QString path =
      QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) +
      "/autostart/";
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
    QList<CaptureTool::Type> buttons =
      CaptureToolButton::getIterableButtonTypes();
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
        if (isShortcut(key)) {
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
    static QSet<QString> options =
      QSet<QString>::fromList(::recognizedGeneralOptions.keys());
    return options;
}

const QSet<QString>& ConfigHandler::recognizedShortcutNames() const
{
    static QSet<QString> names =
      QSet<QString>::fromList(recognizedShortcuts.keys());
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
            keys.insert(baseName(key));
        }
    }
    return keys;
}

// ERROR HANDLING

bool ConfigHandler::checkForErrors(QTextStream* log) const
{
    return checkUnrecognizedSettings(log) & checkShortcutConflicts(log) &
           checkSemantics(log);
}

/**
 * @brief Parse the config to find settings with unrecognized names.
 * @return Whether the config passes this check.
 *
 * @note An unrecognized option is one that is not included in
 * `recognizedGeneralOptions` or `recognizedShortcutNames` depending on the
 * group the option belongs to.
 */
bool ConfigHandler::checkUnrecognizedSettings(QTextStream* log) const
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
    bool ok = generalKeys.isEmpty() && shortcutKeys.isEmpty();
    if (log != nullptr) {
        for (const QString& key : generalKeys) {
            *log << QStringLiteral("Unrecognized setting: '%1'\n").arg(key);
        }
        for (const QString& key : shortcutKeys) {
            *log
              << QStringLiteral("Unrecognized shortcut name: '%1'.\n").arg(key);
        }
    }
    return ok;
}

/**
 * @brief Check if there are multiple shortcuts with the same key binding.
 * @return Whether the config passes this check.
 */
bool ConfigHandler::checkShortcutConflicts(QTextStream* log) const
{
    bool ok = true;
    m_settings.beginGroup("Shortcuts");
    QStringList shortcuts = m_settings.allKeys();
    QStringList reportedInLog;
    for (auto key1 = shortcuts.begin(); key1 != shortcuts.end(); ++key1) {
        for (auto key2 = key1 + 1; key2 != shortcuts.end(); ++key2) {
            // values stored in variables are useful when running debugger
            QString value1 = m_settings.value(*key1).toString(),
                    value2 = m_settings.value(*key2).toString();
            if (!value1.isEmpty() && value1 == value2) {
                ok = false;
                if (log == nullptr) {
                    break;
                } else if (!reportedInLog.contains(*key1) &&
                           !reportedInLog.contains(*key2)) {
                    reportedInLog.append(*key1);
                    reportedInLog.append(*key2);
                    *log << QStringLiteral("Shortcut conflict: '%1' and '%2' "
                                           "have the same shortcut: %3\n")
                              .arg(*key1)
                              .arg(*key2)
                              .arg(value1);
                }
            }
        }
    }
    m_settings.endGroup();
    return ok;
}

/**
 * @brief Check each config value semantically.
 * @return Whether the config passes this check.
 */
bool ConfigHandler::checkSemantics(QTextStream* log) const
{
    QStringList allKeys = m_settings.allKeys();
    bool ok = true;
    for (const QString& key : allKeys) {
        if (!recognizedGeneralOptions().contains(key) &&
            !recognizedShortcutNames().contains(baseName(key))) {
            continue;
        }
        QVariant val = m_settings.value(key);
        auto valueHandler = this->valueHandler(key);
        if (val.isValid() && !valueHandler->check(val)) {
            ok = false;
            if (log == nullptr) {
                break;
            } else {
                *log << QStringLiteral("Semantic error in '%1'. Expected: %2\n")
                          .arg(key)
                          .arg(valueHandler->expected());
            }
        }
    }
    return ok;
}

/**
 * @brief Parse the configuration to find any errors in it.
 *
 * If the error state changes as a result of the check, it will perform the
 * appropriate action, e.g. notify the user.
 *
 * @see ConfigHandler::setErrorState for all the actions.
 */
void ConfigHandler::checkAndHandleError() const
{
    if (!QFile(m_settings.fileName()).exists()) {
        setErrorState(false);
    } else {
        setErrorState(!checkForErrors());
    }

    ensureFileWatched();
}

/**
 * @brief Update the tracked error state of the config.
 * @param error The new error state.
 *
 * The error state is tracked so that signals are not emitted and the user is
 * not spammed every time the config file changes. Instead, only changes in
 * error state get reported.
 */
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

/**
 * @brief Return if the config contains an error.
 *
 * If an error check is due, it will be performed.
 */
bool ConfigHandler::hasError() const
{
    if (m_errorCheckPending) {
        checkAndHandleError();
        m_errorCheckPending = false;
    }
    return m_hasError;
}

/// Error message that can be used by other classes as well
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

/**
 * @brief Obtain a `ValueHandler` for the config option with the given key.
 * @return Smart pointer to the handler.
 *
 * @note If the key is from the "General" group, the `recognizedGeneralOptions`
 * map is looked up. If it is from "Shortcuts", a generic `KeySequence` value
 * handler is returned.
 */
QSharedPointer<ValueHandler> ConfigHandler::valueHandler(
  const QString& key) const
{
    QSharedPointer<ValueHandler> handler;
    if (isShortcut(key)) {
        QString _key = key;
        _key.replace("Shortcuts/", "");
        handler = recognizedShortcuts.value(
          _key, QSharedPointer<KeySequence>(new KeySequence()));
    } else { // General group
        handler = ::recognizedGeneralOptions.value(key);
    }
    return handler;
}

/**
 * This is used so that we can check if there is a mismatch between a config key
 * and its getter function.
 * Debug: throw an exception; Release: set error state
 */
void ConfigHandler::assertKeyRecognized(const QString& key) const
{
    bool recognized = isShortcut(key)
                        ? recognizedShortcutNames().contains(baseName(key))
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
        setErrorState(true);
#endif
    }
}

bool ConfigHandler::isShortcut(const QString& key) const
{
    return m_settings.group() == QStringLiteral("Shortcuts") ||
           key.startsWith(QStringLiteral("Shortcuts/"));
}

QString ConfigHandler::baseName(QString key) const
{
    return QFileInfo(key).baseName();
}

// STATIC MEMBER DEFINITIONS

bool ConfigHandler::m_hasError = false;
bool ConfigHandler::m_errorCheckPending = false;
bool ConfigHandler::m_skipNextErrorCheck = false;
QSharedPointer<QFileSystemWatcher> ConfigHandler::m_configWatcher;
