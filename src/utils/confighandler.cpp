// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "confighandler.h"
#include "tools/capturetool.h"
#include "utils/abstractlogger.h"
#include "utils/valuehandler.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileSystemWatcher>
#include <QHash>
#include <QKeySequence>
#include <QMap>
#include <QSharedPointer>
#include <QStandardPaths>
#include <QThread>
#include <QTimer>
#include <QVector>
#include <memory>
#include <stdexcept>

#if defined(Q_OS_MACOS)
#include <QProcess>
#endif

// HELPER FUNCTIONS

bool verifyLaunchFile()
{
#if defined(Q_OS_UNIX)
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
    OPTION("showAbortNotification"       ,Bool               ( true          )),
    OPTION("disabledTrayIcon"            ,Bool               ( false         )),
    OPTION("historyConfirmationToDelete" ,Bool               ( true          )),
#if !defined(DISABLE_UPDATE_CHECKER)
    OPTION("checkForUpdates"             ,Bool               ( true          )),
#endif
    OPTION("allowMultipleGuiInstances"   ,Bool               ( false         )),
    OPTION("showMagnifier"               ,Bool               ( false         )),
    OPTION("squareMagnifier"             ,Bool               ( false         )),
    OPTION("autoCloseIdleDaemon"         ,Bool               ( false         )),
    OPTION("startupLaunch"               ,Bool               ( false         )),
    OPTION("showStartupLaunchMessage"    ,Bool               ( true          )),
    OPTION("showQuitPrompt"              ,Bool               ( false         )),
    OPTION("copyURLAfterUpload"          ,Bool               ( true          )),
    OPTION("copyPathAfterSave"           ,Bool               ( false         )),
    OPTION("antialiasingPinZoom"         ,Bool               ( true          )),
    OPTION("useJpgForClipboard"          ,Bool               ( false         )),
#if defined(Q_OS_MACOS)
    OPTION("useNativeFullscreen"         ,Bool               ( false         )),
#endif
    OPTION("uploadWithoutConfirmation"   ,Bool               ( false         )),
    OPTION("saveAfterCopy"               ,Bool               ( false         )),
    OPTION("savePath"                    ,ExistingDir        (               )),
    OPTION("savePathFixed"               ,Bool               ( false         )),
    OPTION("saveAsFileExtension"         ,SaveFileExtension  (               )),
    OPTION("saveLastRegion"              ,Bool               ( false         )),
    OPTION("uploadHistoryMax"            ,LowerBoundedInt    ( 0, 25         )),
    OPTION("undoLimit"                   ,BoundedInt         ( 0, 999, 100   )),
    // Interface tab
    OPTION("uiLanguage"                  ,String             ( "auto"        )),
    OPTION("uiColor"                     ,Color              ( {116, 0, 150} )),
    OPTION("contrastUiColor"             ,Color              ( {39, 0, 50}   )),
    OPTION("contrastOpacity"             ,BoundedInt         ( 0, 255, 190   )),
    OPTION("buttons"                     ,ButtonList         ( {}            )),
    // Filename Editor tab
    OPTION("filenamePattern"             ,FilenamePattern    ( {}            )),
    // Others
    // drawThickness shared by Pencil, Line, Arrow, Rectangular Selection, Circle
    OPTION("drawThickness"               ,LowerBoundedInt    ( 1, 3          )),
    OPTION("drawFontSize"                ,LowerBoundedInt    ( 1, 8          )),
    OPTION("drawCircleCounterSize"       ,LowerBoundedInt    ( 1, 1          )),
    OPTION("drawCircleCounterOutline"    ,Bool               ( true          )),
    OPTION("drawPixelateSize"            ,LowerBoundedInt    ( 1, 2          )),
    OPTION("drawRectangleSize"           ,LowerBoundedInt    ( 1, 1          )),
    OPTION("drawMarkerSize"              ,LowerBoundedInt    ( 1, 5          )),
    OPTION("drawColor"                   ,Color              ( Qt::red       )),
    OPTION("userColors"                  ,UserColors         ( 3, 17         )),
    OPTION("ignoreUpdateToVersion"       ,String             ( ""            )),
    OPTION("keepOpenAppLauncher"         ,Bool               ( false         )),
    OPTION("fontFamily"                  ,String             ( ""            )),
    // PREDEFINED_COLOR_PALETTE_LARGE is defined in src/CMakeList.txt file and can be overwritten in GitHub actions
    OPTION("predefinedColorPaletteLarge", Bool               ( PREDEFINED_COLOR_PALETTE_LARGE )),
    // NOTE: If another tool size is added besides drawThickness and
    // drawFontSize, remember to update ConfigHandler::toolSize
    OPTION("copyOnDoubleClick"           ,Bool               ( false         )),
    OPTION("uploadClientSecret"          ,String             ( "313baf0c7b4d3ff" )),
    OPTION("showSelectionGeometry"       , BoundedInt        ( 0, 5, 4       )),
    OPTION("showSelectionGeometryHideTime", LowerBoundedInt  ( 0, 3000       )),
    OPTION("jpegQuality"                 , BoundedInt        ( 0,100,75      )),
    OPTION("reverseArrow"                ,Bool               ( false         )),
    OPTION("arrowStyle"                  ,BoundedInt         ( 0, 1, 0       )),
    OPTION("insecurePixelate"            ,Bool               ( false         )),
#if defined(Q_OS_WIN)
    // Not visible on settings dialog
    OPTION("ignorePrntScrForcesSnipping" ,Bool               ( false         )),
#endif
#if !defined(Q_OS_MACOS)
    // Auto-select the monitor under the cursor instead of showing
    // the monitor selection UI. Not supported on Wayland.
    OPTION("captureActiveMonitor"         ,Bool               ( false         )),
#endif
#if defined(Q_OS_UNIX) && !defined(Q_OS_MACOS)
    // Bypass freedesktop portal and use Qt's native X11
    // screenshot method. Intended for WMs without xdg-desktop-portal.
    OPTION("useX11LegacyScreenshot"      ,Bool               ( false         )),
#endif
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
    SHORTCUT("TYPE_CANCEL"              ,   "Ctrl+Backspace"        ),
#ifdef ENABLE_IMGUR
    SHORTCUT("TYPE_IMAGEUPLOADER"       ,                           ),
#endif
#if !defined(Q_OS_MACOS)
    SHORTCUT("TYPE_OPEN_APP"            ,   "Ctrl+O"                ),
#endif
    SHORTCUT("TYPE_PIXELATE"            ,   "B"                     ),
    SHORTCUT("TYPE_INVERT"              ,   "I"                     ),
    SHORTCUT("TYPE_REDO"                ,   "Ctrl+Shift+Z"          ),
    SHORTCUT("TYPE_TEXT"                ,   "T"                     ),
    SHORTCUT("TYPE_TOGGLE_PANEL"        ,   "Space"                 ),
    SHORTCUT("TYPE_GRAB_COLOR"          ,   "G"                     ),
    SHORTCUT("TYPE_RESIZE_LEFT"         ,   "Shift+Left"            ),
    SHORTCUT("TYPE_RESIZE_RIGHT"        ,   "Shift+Right"           ),
    SHORTCUT("TYPE_RESIZE_UP"           ,   "Shift+Up"              ),
    SHORTCUT("TYPE_RESIZE_DOWN"         ,   "Shift+Down"            ),
    SHORTCUT("TYPE_SYM_RESIZE_LEFT"     ,   "Ctrl+Shift+Left"       ),
    SHORTCUT("TYPE_SYM_RESIZE_RIGHT"    ,   "Ctrl+Shift+Right"      ),
    SHORTCUT("TYPE_SYM_RESIZE_UP"       ,   "Ctrl+Shift+Up"         ),
    SHORTCUT("TYPE_SYM_RESIZE_DOWN"     ,   "Ctrl+Shift+Down"       ),
    SHORTCUT("TYPE_SELECT_ALL"          ,   "Ctrl+A"                ),
    SHORTCUT("TYPE_MOVE_LEFT"           ,   "Left"                  ),
    SHORTCUT("TYPE_MOVE_RIGHT"          ,   "Right"                 ),
    SHORTCUT("TYPE_MOVE_UP"             ,   "Up"                    ),
    SHORTCUT("TYPE_MOVE_DOWN"           ,   "Down"                  ),
    SHORTCUT("TYPE_COMMIT_CURRENT_TOOL" ,   "Ctrl+Return"           ),
#if defined(Q_OS_WIN)
    SHORTCUT("TAKE_SCREENSHOT"          ,   "Meta+Shift+x"          ),
#endif
#if defined(Q_OS_MACOS)
    SHORTCUT("TYPE_DELETE_CURRENT_TOOL" ,   "Backspace"             ),
    SHORTCUT("TAKE_SCREENSHOT"          ,   "Ctrl+Shift+X"          ),
    SHORTCUT("SCREENSHOT_HISTORY"       ,   "Alt+Shift+X"           ),
#else
    SHORTCUT("TYPE_DELETE_CURRENT_TOOL" ,   "Delete"                ),
#endif
    SHORTCUT("TYPE_PIN"                 ,                           ),
    SHORTCUT("TYPE_SIZEINCREASE"        ,                           ),
    SHORTCUT("TYPE_SIZEDECREASE"        ,                           ),
    SHORTCUT("TYPE_CIRCLECOUNT"         ,                           ),
    SHORTCUT("TYPE_CLOSE_ACTIVE_PIN_WINDOW"      ,   "Ctrl+Q"                ),
};
// clang-format on

// CLASS CONFIGHANDLER

namespace {
struct SharedConfigState
{
    QHash<QString, QVariant> rawCache;
    QHash<QString, QVariant> processedCache;
    QHash<QString, QString> shortcutCache;
    QHash<QString, QVariant> explicitShortcuts;
    QHash<QString, QVariant> pendingWrites;
    QSet<QString> pendingRemovals;
    QString cachedFilePath;
    QTimer* syncTimer{ nullptr };
    QElapsedTimer lastSelfSync;
    bool cacheLoaded{ false };
    bool validatedUnrecognizedClean{ false };
    bool validatedSemanticsClean{ false };
    bool validatedConflictsClean{ false };
    bool lastConflictsOk{ true };
    bool pendingClearAll{ false };
    bool expectSelfWrite{ false };
};

SharedConfigState& sharedState()
{
    static SharedConfigState state;
    return state;
}

std::unique_ptr<QSettings> makeSettings()
{
#ifndef USE_PORTABLE_CONFIG
    return std::make_unique<QSettings>(QSettings::IniFormat,
                                       QSettings::UserScope,
                                       qApp->organizationName(),
                                       qApp->applicationName());
#else
    return std::make_unique<QSettings>(qApp->applicationDirPath() +
                                         QStringLiteral("/flameshot.ini"),
                                       QSettings::IniFormat);
#endif
}

void invalidateCache()
{
    auto& state = sharedState();
    state.cacheLoaded = false;
    state.validatedUnrecognizedClean = false;
    state.validatedSemanticsClean = false;
    state.validatedConflictsClean = false;
    state.rawCache.clear();
    state.processedCache.clear();
    state.shortcutCache.clear();
    state.explicitShortcuts.clear();
}

void ensureCacheLoaded()
{
    auto& state = sharedState();
    if (state.cacheLoaded) {
        return;
    }
    state.rawCache.clear();
    state.processedCache.clear();
    state.shortcutCache.clear();
    state.explicitShortcuts.clear();

    if (qApp) {
        auto settings = makeSettings();
        if (state.cachedFilePath.isEmpty()) {
            state.cachedFilePath = settings->fileName();
        }
        const QStringList keys = settings->allKeys();
        for (const QString& key : keys) {
            const QVariant val = settings->value(key);
            state.rawCache.insert(key, val);
            if (key.startsWith(QStringLiteral(CONFIG_GROUP_SHORTCUTS "/"))) {
                state.explicitShortcuts.insert(key, val);
            }
        }
    }
    state.cacheLoaded = true;
}

void flushSync(bool suppressReload)
{
    auto& state = sharedState();
    if (state.syncTimer) {
        state.syncTimer->stop();
    }
    if (!state.pendingClearAll && state.pendingRemovals.isEmpty() &&
        state.pendingWrites.isEmpty()) {
        return;
    }
    if (!qApp) {
        return;
    }

    auto settings = makeSettings();
    if (state.pendingClearAll) {
        for (const QString& key : settings->allKeys()) {
            if (!key.startsWith(QStringLiteral(CONFIG_GROUP_SHORTCUTS "/"))) {
                settings->remove(key);
            }
        }
        state.pendingClearAll = false;
    }
    for (const QString& key : state.pendingRemovals) {
        settings->remove(key);
    }
    state.pendingRemovals.clear();
    for (auto it = state.pendingWrites.constBegin();
         it != state.pendingWrites.constEnd();
         ++it) {
        settings->setValue(it.key(), it.value());
    }
    state.pendingWrites.clear();

    state.expectSelfWrite = suppressReload;
    if (suppressReload) {
        state.lastSelfSync.start();
    }
    settings->sync();
}

void scheduleSync()
{
    auto& state = sharedState();
    if (!qApp || !QThread::currentThread() ||
        QThread::currentThread()->loopLevel() == 0) {
        flushSync(true);
        return;
    }
    if (!state.syncTimer) {
        auto* timer = new QTimer(qApp);
        timer->setSingleShot(true);
        timer->setInterval(150);
        QObject::connect(
          timer, &QTimer::timeout, timer, []() { flushSync(true); });
        QObject::connect(timer, &QObject::destroyed, []() {
            sharedState().syncTimer = nullptr;
            flushSync(false);
        });
        QObject::connect(qApp, &QCoreApplication::aboutToQuit, timer, []() {
            flushSync(false);
        });
        state.syncTimer = timer;
    }
    state.syncTimer->start();
}
} // namespace

ConfigHandler::ConfigHandler()
{
    static bool firstInitialization = true;
    if (firstInitialization) {
        // check for error every time the file changes
        m_configWatcher.reset(new QFileSystemWatcher());
        ensureFileWatched();
        QObject::connect(m_configWatcher.data(),
                         &QFileSystemWatcher::fileChanged,
                         [](const QString& fileName) {
                             auto& state = sharedState();
                             const bool wasSelfWrite =
                               state.expectSelfWrite &&
                               state.lastSelfSync.isValid() &&
                               state.lastSelfSync.elapsed() < 200;
                             state.expectSelfWrite = false;
                             if (!wasSelfWrite) {
                                 invalidateCache();
                             }

                             emit getInstance()->fileChanged();

                             if (QFile(fileName).exists()) {
                                 m_configWatcher->addPath(fileName);
                             }
                             if (m_skipNextErrorCheck || wasSelfWrite) {
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
    firstInitialization = false;
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
#elif defined(Q_OS_UNIX)
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

void ConfigHandler::setAllTheButtons()
{
    QList<CaptureTool::Type> buttonlist =
      CaptureToolButton::getIterableButtonTypes();
    setValue(QStringLiteral("buttons"), QVariant::fromValue(buttonlist));
}

void ConfigHandler::setToolSize(CaptureTool::Type toolType, int size)
{
    if (toolType == CaptureTool::TYPE_TEXT) {
        setDrawFontSize(size);
    } else if (toolType == CaptureTool::TYPE_RECTANGLE) {
        setDrawRectangleSize(size);
    } else if (toolType == CaptureTool::TYPE_MARKER) {
        setDrawMarkerSize(size);
    } else if (toolType == CaptureTool::TYPE_PIXELATE) {
        setDrawPixelateSize(size);
    } else if (toolType == CaptureTool::TYPE_CIRCLECOUNT) {
        setDrawCircleCounterSize(size);
    } else {
        // All other tools are sharing the same size. CaptureTool::NONE lands
        // here too: no tool button is active, but the size still belongs to
        // the shared bucket, which is what toolSize() returns for NONE.
        setDrawThickness(size);
    }
}

int ConfigHandler::toolSize(CaptureTool::Type toolType)
{
    if (toolType == CaptureTool::TYPE_TEXT) {
        return drawFontSize();
    } else if (toolType == CaptureTool::TYPE_RECTANGLE) {
        return drawRectangleSize();
    } else if (toolType == CaptureTool::TYPE_MARKER) {
        return drawMarkerSize();
    } else if (toolType == CaptureTool::TYPE_PIXELATE) {
        return drawPixelateSize();
    } else if (toolType == CaptureTool::TYPE_CIRCLECOUNT) {
        return drawCircleCounterSize();
    } else {
        // All other tools are sharing the same size
        return drawThickness();
    }
}

// DEFAULTS

QString ConfigHandler::filenamePatternDefault()
{
    return QStringLiteral("%F_%H-%M");
}

void ConfigHandler::setDefaultSettings()
{
    auto& state = sharedState();
    ensureCacheLoaded();

    const QString shortcutPrefix = QStringLiteral(CONFIG_GROUP_SHORTCUTS "/");
    for (auto it = state.rawCache.begin(); it != state.rawCache.end();) {
        if (!it.key().startsWith(shortcutPrefix)) {
            state.processedCache.remove(it.key());
            it = state.rawCache.erase(it);
        } else {
            ++it;
        }
    }
    for (auto it = state.pendingWrites.begin();
         it != state.pendingWrites.end();) {
        if (!it.key().startsWith(shortcutPrefix)) {
            it = state.pendingWrites.erase(it);
        } else {
            ++it;
        }
    }
    state.pendingClearAll = true;
    state.validatedUnrecognizedClean = false;
    state.validatedSemanticsClean = false;
    m_skipNextErrorCheck = false;
    flushSync(false);
}

QString ConfigHandler::configFilePath() const
{
    auto& state = sharedState();
    if (state.cachedFilePath.isEmpty() && qApp) {
        state.cachedFilePath = makeSettings()->fileName();
    }
    return state.cachedFilePath;
}

// GENERIC GETTERS AND SETTERS

bool ConfigHandler::setShortcut(const QString& actionName,
                                const QString& shortcut)
{
    qDebug() << actionName;
    static QVector<QKeySequence> reservedShortcuts = {
#if defined(Q_OS_MACOS)
        Qt::CTRL | Qt::Key_Backspace,
        Qt::Key_Escape,
#else
        Qt::Key_Backspace,
        Qt::Key_Escape,
#endif
    };

    if (hasError()) {
        return false;
    }

    bool errorFlag = false;
    const QString fullKey =
      QStringLiteral(CONFIG_GROUP_SHORTCUTS "/") + actionName;

    if (shortcut.isEmpty()) {
        setValue(fullKey, QString());
    } else if (reservedShortcuts.contains(QKeySequence(shortcut))) {
        // do not allow to set reserved shortcuts
        errorFlag = true;
    } else {
        auto& state = sharedState();
        ensureCacheLoaded();

        // Make no difference for Return and Enter keys
        const QString newShortcut = KeySequence().value(shortcut).toString();
        for (auto it = state.explicitShortcuts.constBegin();
             it != state.explicitShortcuts.constEnd();
             ++it) {
            if (it.key() == fullKey) {
                continue;
            }
            const QString existingShortcut =
              KeySequence().value(it.value()).toString();
            if (newShortcut == existingShortcut) {
                errorFlag = true;
                break;
            }
        }
        if (!errorFlag) {
            const QVariant encoded = KeySequence().value(shortcut);
            m_skipNextErrorCheck = true;
            state.rawCache.insert(fullKey, encoded);
            state.processedCache.remove(fullKey);
            state.explicitShortcuts.insert(fullKey, encoded);
            state.shortcutCache.clear();
            state.validatedConflictsClean = false;
            state.pendingRemovals.remove(fullKey);
            state.pendingWrites.insert(fullKey, encoded);
            scheduleSync();
        }
    }
    return !errorFlag;
}

QString ConfigHandler::shortcut(const QString& actionName)
{
    auto& state = sharedState();
    ensureCacheLoaded();

    auto cachedIt = state.shortcutCache.constFind(actionName);
    if (cachedIt != state.shortcutCache.constEnd()) {
        return cachedIt.value();
    }

    const QString setting =
      QStringLiteral(CONFIG_GROUP_SHORTCUTS "/") + actionName;
    QString shortcut = value(setting).toString();
    if (!state.rawCache.contains(setting)) {
        // The action uses a shortcut that is a flameshot default
        // (not set explicitly by user)
        for (auto it = state.explicitShortcuts.constBegin();
             it != state.explicitShortcuts.constEnd();
             ++it) {
            if (it.value().toString() == shortcut) {
                // We found an explicit shortcut - it will take precedence
                shortcut.clear();
                break;
            }
        }
    }
    state.shortcutCache.insert(actionName, shortcut);
    return shortcut;
}

void ConfigHandler::setValue(const QString& key, const QVariant& value)
{
    assertKeyRecognized(key);
    if (!hasError()) {
        auto& state = sharedState();
        ensureCacheLoaded();

        // don't let the file watcher initiate another error check
        m_skipNextErrorCheck = true;
        auto val = valueHandler(key)->representation(value);
        state.rawCache.insert(key, val);
        state.processedCache.remove(key);
        if (isShortcut(key)) {
            state.explicitShortcuts.insert(key, val);
            state.shortcutCache.clear();
            state.validatedConflictsClean = false;
        }
        state.pendingRemovals.remove(key);
        state.pendingWrites.insert(key, val);
        scheduleSync();
    }
}

QVariant ConfigHandler::value(const QString& key) const
{
    assertKeyRecognized(key);

    auto& state = sharedState();
    ensureCacheLoaded();

    if (!m_hasError) {
        auto it = state.processedCache.constFind(key);
        if (it != state.processedCache.constEnd()) {
            return it.value();
        }
    }

    const QVariant val = state.rawCache.value(key);
    auto handler = valueHandler(key);

    // Check the value for semantic errors
    if (val.isValid() && !handler->check(val)) {
        setErrorState(true);
    }
    if (m_hasError) {
        return handler->fallback();
    }

    const QVariant processed = handler->value(val);
    state.processedCache.insert(key, processed);
    return processed;
}

void ConfigHandler::remove(const QString& key)
{
    auto& state = sharedState();
    ensureCacheLoaded();

    state.rawCache.remove(key);
    state.processedCache.remove(key);
    if (isShortcut(key)) {
        state.explicitShortcuts.remove(key);
        state.shortcutCache.clear();
        state.validatedConflictsClean = false;
    }
    state.validatedUnrecognizedClean = false;
    state.validatedSemanticsClean = false;
    state.pendingWrites.remove(key);
    state.pendingRemovals.insert(key);
    m_skipNextErrorCheck = false;
    flushSync(false);
}

void ConfigHandler::resetValue(const QString& key)
{
    auto& state = sharedState();
    ensureCacheLoaded();

    const QVariant fb = valueHandler(key)->fallback();
    state.rawCache.insert(key, fb);
    state.processedCache.remove(key);
    if (isShortcut(key)) {
        state.explicitShortcuts.insert(key, fb);
        state.shortcutCache.clear();
        state.validatedConflictsClean = false;
    }
    state.validatedUnrecognizedClean = false;
    state.validatedSemanticsClean = false;
    state.pendingRemovals.remove(key);
    state.pendingWrites.insert(key, fb);
    m_skipNextErrorCheck = false;
    flushSync(false);
}

QSet<QString>& ConfigHandler::recognizedGeneralOptions()
{
    auto keys = ::recognizedGeneralOptions.keys();
    static QSet<QString> options = QSet<QString>(keys.begin(), keys.end());
    return options;
}

QSet<QString>& ConfigHandler::recognizedShortcutNames()
{
    auto keys = recognizedShortcuts.keys();
    static QSet<QString> names = QSet<QString>(keys.begin(), keys.end());
    return names;
}

/**
 * @brief Return keys from group `group`.
 * Use CONFIG_GROUP_GENERAL (General) for general settings.
 */
QSet<QString> ConfigHandler::keysFromGroup(const QString& group) const
{
    auto& state = sharedState();
    ensureCacheLoaded();

    QSet<QString> keys;
    const QString prefix = group + "/";
    for (auto it = state.rawCache.constBegin(); it != state.rawCache.constEnd();
         ++it) {
        const QString& key = it.key();
        if (group == CONFIG_GROUP_GENERAL && !key.contains('/')) {
            keys.insert(key);
        } else if (key.startsWith(prefix)) {
            keys.insert(baseName(key));
        }
    }
    return keys;
}

// ERROR HANDLING

bool ConfigHandler::checkForErrors(AbstractLogger* log) const
{
    return checkUnrecognizedSettings(log) && checkShortcutConflicts(log) &&
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
bool ConfigHandler::checkUnrecognizedSettings(AbstractLogger* log,
                                              QList<QString>* offenders) const
{
    auto& state = sharedState();
    ensureCacheLoaded();
    if (log == nullptr && offenders == nullptr &&
        state.validatedUnrecognizedClean) {
        return true;
    }

    // sort the config keys by group
    QSet<QString> generalKeys = keysFromGroup(CONFIG_GROUP_GENERAL),
                  shortcutKeys = keysFromGroup(CONFIG_GROUP_SHORTCUTS),
                  recognizedGeneralKeys = recognizedGeneralOptions(),
                  recognizedShortcutKeys = recognizedShortcutNames();

    // subtract recognized keys
    generalKeys.subtract(recognizedGeneralKeys);
    shortcutKeys.subtract(recognizedShortcutKeys);

    // what is left are the unrecognized keys - hopefully empty
    bool ok = generalKeys.isEmpty() && shortcutKeys.isEmpty();
    if (ok) {
        state.validatedUnrecognizedClean = true;
    }
    if (log != nullptr || offenders != nullptr) {
        for (const QString& key : generalKeys) {
            if (log) {
                *log << tr("Unrecognized setting: '%1'\n").arg(key);
            }
            if (offenders) {
                offenders->append(key);
            }
        }
        for (const QString& key : shortcutKeys) {
            if (log) {
                *log << tr("Unrecognized shortcut name: '%1'.\n").arg(key);
            }
            if (offenders) {
                offenders->append(CONFIG_GROUP_SHORTCUTS "/" + key);
            }
        }
    }
    return ok;
}

/**
 * @brief Check if there are multiple actions with the same shortcut.
 * @return Whether the config passes this check.
 *
 * @note It is not considered a conflict if action A uses shortcut S because it
 * is the flameshot default (not because the user explicitly configured it), and
 * action B uses the same shortcut.
 */
bool ConfigHandler::checkShortcutConflicts(AbstractLogger* log) const
{
    auto& state = sharedState();
    ensureCacheLoaded();
    if (log == nullptr && state.validatedConflictsClean) {
        return state.lastConflictsOk;
    }

    bool ok = true;
    QStringList shortcuts;
    shortcuts.reserve(state.explicitShortcuts.size());
    for (auto it = state.explicitShortcuts.constBegin();
         it != state.explicitShortcuts.constEnd();
         ++it) {
        shortcuts.append(it.key());
    }
    QStringList reportedInLog;
    for (auto key1 = shortcuts.begin(); key1 != shortcuts.end(); ++key1) {
        for (auto key2 = key1 + 1; key2 != shortcuts.end(); ++key2) {
            // values stored in variables are useful when running debugger
            QString value1 = state.explicitShortcuts.value(*key1).toString(),
                    value2 = state.explicitShortcuts.value(*key2).toString();
            // The check will pass if:
            // - one shortcut is empty (the action doesn't use a shortcut)
            // - or one of the settings is not found in cache
            // - or the shortcuts for both actions are different
            if (!(value1.isEmpty() || value1 != value2)) {
                ok = false;
                if (log == nullptr) {
                    break;
                }
                const QString name1 = baseName(*key1);
                const QString name2 = baseName(*key2);
                if (!reportedInLog.contains(name1) && // No duplicate
                    !reportedInLog.contains(name2)) { // log entries
                    reportedInLog.append(name1);
                    reportedInLog.append(name2);
                    *log << tr("Shortcut conflict: '%1' and '%2' "
                               "have the same shortcut: %3\n")
                              .arg(name1, name2, value1);
                }
            }
        }
    }
    state.validatedConflictsClean = true;
    state.lastConflictsOk = ok;
    return ok;
}

/**
 * @brief Check each config value semantically.
 * @param log Destination for error log output.
 * @param offenders Destination for the semantically invalid keys.
 * @return Whether the config passes this check.
 */
bool ConfigHandler::checkSemantics(AbstractLogger* log,
                                   QList<QString>* offenders) const
{
    auto& state = sharedState();
    ensureCacheLoaded();
    if (log == nullptr && offenders == nullptr &&
        state.validatedSemanticsClean) {
        return true;
    }

    bool ok = true;
    for (auto it = state.rawCache.constBegin(); it != state.rawCache.constEnd();
         ++it) {
        const QString& key = it.key();
        // Test if the key is recognized
        if (!recognizedGeneralOptions().contains(key) &&
            (!isShortcut(key) ||
             !recognizedShortcutNames().contains(baseName(key)))) {
            continue;
        }
        const QVariant& val = it.value();
        auto valueHandler = this->valueHandler(key);
        if (val.isValid() && !valueHandler->check(val)) {
            // Key does not pass the check
            ok = false;
            if (log == nullptr && offenders == nullptr) {
                break;
            }
            if (log != nullptr) {
                *log << tr("Bad value in '%1'. Expected: %2\n")
                          .arg(key, valueHandler->expected());
            }
            if (offenders != nullptr) {
                offenders->append(key);
            }
        }
    }
    if (ok) {
        state.validatedSemanticsClean = true;
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
    if (!QFile(configFilePath()).exists()) {
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
        AbstractLogger::error() << msg;
        emit getInstance()->error();
    } else if (hadError && !m_hasError) {
        auto msg =
          tr("You have successfully resolved the configuration error.");
        AbstractLogger::info() << msg;
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
    return tr(
      "The configuration contains an error. Open configuration to resolve.");
}

void ConfigHandler::ensureFileWatched() const
{
    const QString path = configFilePath();
    if (path.isEmpty()) {
        return;
    }
    QFile file(path);
    if (!file.exists()) {
        if (file.open(QFileDevice::WriteOnly)) {
            file.close();
        }
    }
    if (m_configWatcher != nullptr && m_configWatcher->files().isEmpty() &&
        qApp != nullptr // ensures that the organization name can be accessed
    ) {
        m_configWatcher->addPath(path);
    }
}

/**
 * @brief Obtain a `ValueHandler` for the config option with the given key.
 * @return Smart pointer to the handler.
 *
 * @note If the key is from the CONFIG_GROUP_GENERAL (General) group, the
 * `recognizedGeneralOptions` map is looked up. If it is from
 * CONFIG_GROUP_SHORTCUTS (Shortcuts), a generic `KeySequence` value handler is
 * returned.
 */
QSharedPointer<ValueHandler> ConfigHandler::valueHandler(
  const QString& key) const
{
    QSharedPointer<ValueHandler> handler;
    if (isShortcut(key)) {
        handler = recognizedShortcuts.value(
          baseName(key), QSharedPointer<KeySequence>(new KeySequence()));
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
    return key.startsWith(QStringLiteral(CONFIG_GROUP_SHORTCUTS "/"));
}

QString ConfigHandler::baseName(const QString& key) const
{
    return QFileInfo(key).baseName();
}

// STATIC MEMBER DEFINITIONS

bool ConfigHandler::m_hasError = false;
bool ConfigHandler::m_errorCheckPending = true;
bool ConfigHandler::m_skipNextErrorCheck = false;

QSharedPointer<QFileSystemWatcher> ConfigHandler::m_configWatcher;
