// SPDX-License-Identifier: GPL-3.0-or-later

#include "actionplugin.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSet>
#include <QStandardPaths>

#include <algorithm>

namespace {

bool fail(QString* error, const QString& message)
{
    if (error) {
        *error = message;
    }
    return false;
}

QString resolveExecutable(const QString& value, const QString& pluginDir)
{
    if (value.isEmpty()) {
        return {};
    }

    const QFileInfo executableInfo(value);
    if (executableInfo.isAbsolute()) {
        return executableInfo.absoluteFilePath();
    }

    if (value.contains(QLatin1Char('/')) || value.contains(QLatin1Char('\\'))) {
        const QString normalized =
          QString(value).replace(QLatin1Char('\\'), QLatin1Char('/'));
        const QString clean = QDir::cleanPath(normalized);
        if (clean == QLatin1String("..") ||
            clean.startsWith(QLatin1String("../"))) {
            return {};
        }
        return QDir(pluginDir).absoluteFilePath(clean);
    }

    QString executable = QStandardPaths::findExecutable(value, { pluginDir });
    if (executable.isEmpty()) {
        executable = QStandardPaths::findExecutable(value);
    }
    return executable;
}

} // namespace

QString ActionPluginLoader::userPluginRoot()
{
    return QDir(QStandardPaths::writableLocation(
                  QStandardPaths::GenericDataLocation))
      .filePath(QStringLiteral("flameshot/plugins"));
}

QStringList ActionPluginLoader::pluginRoots()
{
    QStringList roots;
    const QStringList dataLocations =
      QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    for (const QString& location : dataLocations) {
        roots.append(
          QDir(location).filePath(QStringLiteral("flameshot/plugins")));
    }

    // Compatibility with the first prototype and the path discussed in the
    // original plugin RFC.
    const QString legacyRoot = QDir(QStandardPaths::writableLocation(
                                      QStandardPaths::GenericConfigLocation))
                                 .filePath(QStringLiteral("flameshot/plugins"));
    if (!roots.contains(legacyRoot)) {
        roots.append(legacyRoot);
    }
    roots.removeDuplicates();
    return roots;
}

QList<ActionPlugin> ActionPluginLoader::discover(const QStringList& pluginRoots,
                                                 QStringList* errors)
{
    QList<ActionPlugin> plugins;
    QSet<QString> knownIds;

    for (const QString& pluginRoot : pluginRoots) {
        QDirIterator manifests(pluginRoot,
                               { QStringLiteral("metadata.json") },
                               QDir::Files,
                               QDirIterator::Subdirectories);
        while (manifests.hasNext()) {
            ActionPlugin plugin;
            QString error;
            const QString manifestPath = manifests.next();
            if (!loadManifest(manifestPath, &plugin, &error)) {
                if (errors) {
                    errors->append(
                      QStringLiteral("%1: %2").arg(manifestPath, error));
                }
                continue;
            }
            if (knownIds.contains(plugin.id)) {
                if (errors) {
                    errors->append(QStringLiteral("%1: duplicate plugin id %2")
                                     .arg(manifestPath, plugin.id));
                }
                continue;
            }
            knownIds.insert(plugin.id);
            plugins.append(plugin);
        }
    }

    std::sort(
      plugins.begin(), plugins.end(), [](const auto& left, const auto& right) {
          return left.name.localeAwareCompare(right.name) < 0;
      });
    return plugins;
}

bool ActionPluginLoader::loadManifest(const QString& path,
                                      ActionPlugin* plugin,
                                      QString* error)
{
    if (!plugin) {
        return fail(error, QStringLiteral("missing output object"));
    }
    *plugin = ActionPlugin{};

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return fail(error, file.errorString());
    }

    QJsonParseError parseError;
    const QJsonDocument document =
      QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        return fail(
          error,
          QStringLiteral("invalid JSON: %1").arg(parseError.errorString()));
    }

    const QJsonObject root = document.object();
    if (root.value("api_version").toInt(-1) != ApiVersion) {
        return fail(error,
                    QStringLiteral("unsupported api_version (expected %1)")
                      .arg(ApiVersion));
    }
    if (root.value("type").toString() != QLatin1String("action")) {
        return fail(error, QStringLiteral("type must be 'action'"));
    }

    static const QRegularExpression validId(
      QStringLiteral("^[a-z0-9]+(?:[._-][a-z0-9]+)+$"));
    plugin->id = root.value("id").toString();
    if (!validId.match(plugin->id).hasMatch()) {
        return fail(error, QStringLiteral("invalid plugin id"));
    }

    plugin->name = root.value("name").toString().trimmed();
    if (plugin->name.isEmpty()) {
        return fail(error, QStringLiteral("name is required"));
    }
    plugin->description = root.value("description").toString().trimmed();

    const QJsonObject command = root.value("command").toObject();
    const QString pluginDir = QFileInfo(path).absolutePath();
    plugin->executable =
      resolveExecutable(command.value("executable").toString(), pluginDir);
    const QFileInfo resolvedExecutable(plugin->executable);
    if (plugin->executable.isEmpty() || !resolvedExecutable.isExecutable()) {
        return fail(error, QStringLiteral("executable was not found"));
    }

    const QJsonArray arguments = command.value("arguments").toArray();
    for (const QJsonValue& argument : arguments) {
        if (!argument.isString()) {
            return fail(error,
                        QStringLiteral("command arguments must be strings"));
        }
        plugin->arguments.append(argument.toString());
    }

    if (command.value("input").toString("png-stdin") !=
        QLatin1String("png-stdin")) {
        return fail(error, QStringLiteral("only png-stdin input is supported"));
    }

    const QString output = command.value("output").toString("none");
    if (output == QLatin1String("clipboard-text")) {
        plugin->outputMode = ActionPlugin::OutputMode::ClipboardText;
    } else if (output == QLatin1String("json")) {
        plugin->outputMode = ActionPlugin::OutputMode::Json;
    } else if (output == QLatin1String("none")) {
        plugin->outputMode = ActionPlugin::OutputMode::None;
    } else {
        return fail(error, QStringLiteral("unsupported command output"));
    }

    plugin->timeoutMs = command.value("timeout_ms").toInt(30000);
    if (plugin->timeoutMs < 1000 || plugin->timeoutMs > 300000) {
        return fail(
          error, QStringLiteral("timeout_ms must be between 1000 and 300000"));
    }

    const QJsonObject toolbar = root.value("toolbar").toObject();
    plugin->toolbarVisible = toolbar.value("visible").toBool(true);
    plugin->toolbarOrder = toolbar.value("order").toInt(1000);
    if (plugin->toolbarOrder < 0 || plugin->toolbarOrder > 100000) {
        return fail(
          error, QStringLiteral("toolbar.order must be between 0 and 100000"));
    }
    plugin->shortcut = toolbar.value("shortcut").toString();

    const QString iconName = root.value("icon").toString();
    if (!iconName.isEmpty()) {
        const QString cleanIcon = QDir::cleanPath(
          QString(iconName).replace(QLatin1Char('\\'), QLatin1Char('/')));
        if (QDir::isAbsolutePath(cleanIcon) ||
            cleanIcon == QLatin1String("..") ||
            cleanIcon.startsWith(QLatin1String("../"))) {
            return fail(error, QStringLiteral("invalid icon path"));
        }
        const QString iconPath = QDir(pluginDir).absoluteFilePath(cleanIcon);
        if (QFileInfo::exists(iconPath)) {
            plugin->icon = QIcon(iconPath);
        }
    }
    plugin->manifestPath = QFileInfo(path).absoluteFilePath();
    return true;
}
