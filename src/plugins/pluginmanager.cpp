// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#include "pluginmanager.h"
#include "tools/capturetool.h"
#include "utils/confighandler.h"

#include <QCoreApplication>
#include <QDir>
#include <QProcessEnvironment>
#include <QSet>
#include <QStandardPaths>

PluginManager* PluginManager::m_instance = nullptr;

PluginManager* PluginManager::instance()
{
    if (!m_instance) {
        m_instance = new PluginManager(qApp);
    }
    return m_instance;
}

PluginManager::PluginManager(QObject* parent)
  : QObject(parent)
{
    // Retrieve disabled plugins from configuration
    m_disabledPluginIds = ConfigHandler().disabledPlugins();
    scanAndLoadPlugins();
}

PluginManager::~PluginManager()
{
    unloadPlugins();
}

QString PluginManager::userPluginsDirectory() const
{
    QString base =
      QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (base.isEmpty()) {
        base = QDir::homePath() + QStringLiteral("/.local/share/flameshot");
    }
    QString dirPath = base + QStringLiteral("/plugins");
    QDir().mkpath(dirPath);
    return dirPath;
}

QStringList PluginManager::pluginSearchPaths() const
{
    QStringList paths;

    // 1. Check environment override
    QString envPath = QProcessEnvironment::systemEnvironment().value(
      QStringLiteral("FLAMESHOT_PLUGIN_PATH"));
    if (!envPath.isEmpty()) {
#if defined(Q_OS_WIN)
        paths.append(envPath.split(QLatin1Char(';'), Qt::SkipEmptyParts));
#else
        paths.append(envPath.split(QLatin1Char(':'), Qt::SkipEmptyParts));
#endif
    }

    // 2. User directory (~/.local/share/flameshot/plugins or similar)
    paths.append(userPluginsDirectory());

    // 3. User config directory fallback
    QString configDir =
      QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) +
      QStringLiteral("/plugins");
    if (!paths.contains(configDir)) {
        paths.append(configDir);
    }

    // 4. System-wide directories
#if defined(APP_PREFIX)
    QString sysPath =
      QStringLiteral(APP_PREFIX) + QStringLiteral("/lib/flameshot/plugins");
    if (!paths.contains(sysPath)) {
        paths.append(sysPath);
    }
#endif

    // 5. Binary relative directory
    QString appDir =
      QCoreApplication::applicationDirPath() + QStringLiteral("/plugins");
    if (!paths.contains(appDir)) {
        paths.append(appDir);
    }

    return paths;
}

void PluginManager::scanAndLoadPlugins()
{
    unloadPlugins();
    m_disabledPluginIds = ConfigHandler().disabledPlugins();

    QStringList searchPaths = pluginSearchPaths();
    QSet<QString> loadedIds;

    for (const QString& dirPath : searchPaths) {
        QDir dir(dirPath);
        if (!dir.exists()) {
            continue;
        }

        QStringList filters;
#if defined(Q_OS_WIN)
        filters << QStringLiteral("*.dll");
#elif defined(Q_OS_MACOS)
        filters << QStringLiteral("*.dylib") << QStringLiteral("*.so");
#else
        filters << QStringLiteral("*.so");
#endif

        QFileInfoList fileList =
          dir.entryInfoList(filters, QDir::Files | QDir::Readable);
        for (const QFileInfo& fileInfo : fileList) {
            loadPluginFromFile(fileInfo.absoluteFilePath());
        }
    }

    emit pluginsChanged();
}

void PluginManager::loadPluginFromFile(const QString& path)
{
    auto* loader = new QPluginLoader(path, this);
    QObject* rootObj = loader->instance();
    if (!rootObj) {
        loader->unload();
        delete loader;
        return;
    }

    auto* plugin = qobject_cast<FlameshotPluginInterface*>(rootObj);
    if (!plugin) {
        loader->unload();
        delete loader;
        return;
    }

    QString id = plugin->pluginId();
    // Prevent loading duplicate IDs from multiple search directories
    for (const auto& existing : m_plugins) {
        if (existing.id == id) {
            loader->unload();
            delete loader;
            return;
        }
    }

    PluginMetadata meta;
    meta.filePath = path;
    meta.fileName = QFileInfo(path).fileName();
    meta.id = id;
    meta.name = plugin->pluginName();
    meta.version = plugin->pluginVersion();
    meta.author = plugin->pluginAuthor();
    meta.description = plugin->pluginDescription();
    meta.icon = plugin->pluginIcon();
    meta.enabled = !m_disabledPluginIds.contains(id);
    meta.instance = plugin;
    meta.loader = loader;

    if (meta.enabled) {
        plugin->onPluginLoaded();
    }

    m_plugins.append(meta);
}

void PluginManager::unloadPlugins()
{
    for (auto& meta : m_plugins) {
        if (meta.instance && meta.enabled) {
            meta.instance->onPluginUnloaded();
        }
        if (meta.loader) {
            meta.loader->unload();
            delete meta.loader;
            meta.loader = nullptr;
        }
    }
    m_plugins.clear();
}

void PluginManager::reloadPlugins()
{
    scanAndLoadPlugins();
}

const QVector<PluginMetadata>& PluginManager::plugins() const
{
    return m_plugins;
}

QList<CaptureTool*> PluginManager::createPluginTools(QObject* parent)
{
    QList<CaptureTool*> tools;
    for (const auto& meta : m_plugins) {
        if (meta.enabled && meta.instance) {
            tools.append(meta.instance->createTools(parent));
        }
    }
    return tools;
}

bool PluginManager::isPluginEnabled(const QString& id) const
{
    for (const auto& meta : m_plugins) {
        if (meta.id == id) {
            return meta.enabled;
        }
    }
    return false;
}

void PluginManager::setPluginEnabled(const QString& id, bool enabled)
{
    for (auto& meta : m_plugins) {
        if (meta.id == id) {
            if (meta.enabled == enabled) {
                return;
            }
            meta.enabled = enabled;
            if (enabled) {
                m_disabledPluginIds.removeAll(id);
                if (meta.instance) {
                    meta.instance->onPluginLoaded();
                }
            } else {
                if (!m_disabledPluginIds.contains(id)) {
                    m_disabledPluginIds.append(id);
                }
                if (meta.instance) {
                    meta.instance->onPluginUnloaded();
                }
            }
            ConfigHandler().setDisabledPlugins(m_disabledPluginIds);
            emit pluginsChanged();
            return;
        }
    }
}
