// SPDX-License-Identifier: GPL-3.0-or-later

#include "plugins/actionplugin.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>

#include <iostream>

namespace {

bool writeManifest(const QString& path, const QJsonObject& manifest)
{
    QFile file(path);
    return file.open(QIODevice::WriteOnly) &&
           file.write(QJsonDocument(manifest).toJson()) > 0;
}

QJsonObject validManifest(const QString& executable)
{
    return {
        { "api_version", 1 },
        { "type", "action" },
        { "id", "org.flameshot.test-action" },
        { "name", "Test action" },
        { "toolbar",
          QJsonObject{
            { "visible", true },
            { "order", 175 },
            { "shortcut", "Ctrl+Shift+T" },
          } },
        { "command",
          QJsonObject{
            { "executable", executable },
            { "arguments", QJsonArray{ "-E", "true" } },
            { "input", "png-stdin" },
            { "output", "clipboard-text" },
            { "timeout_ms", 5000 },
          } },
    };
}

bool expect(bool condition, const char* message)
{
    if (!condition) {
        std::cerr << message << '\n';
    }
    return condition;
}

} // namespace

int main(int argc, char* argv[])
{
    if (argc != 2) {
        std::cerr << "expected path to a test executable\n";
        return 1;
    }

    QTemporaryDir directory;
    if (!expect(directory.isValid(), "could not create temporary directory")) {
        return 1;
    }
    const QString configHome = directory.filePath("config");
    const QString dataHome = directory.filePath("data");
    qputenv("XDG_CONFIG_HOME", configHome.toUtf8());
    qputenv("XDG_DATA_HOME", dataHome.toUtf8());
    qputenv("XDG_DATA_DIRS", "/usr/local/share:/usr/share");

    QCoreApplication application(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("flameshot"));
    QCoreApplication::setOrganizationName(QStringLiteral("flameshot"));
    const QString path = directory.filePath("metadata.json");

    if (!expect(ActionPluginLoader::userPluginRoot() ==
                  dataHome + QLatin1String("/flameshot/plugins"),
                "user plugin root does not follow XDG_DATA_HOME") ||
        !expect(ActionPluginLoader::pluginRoots().contains(
                  dataHome + QLatin1String("/flameshot/plugins")),
                "XDG user data root is missing") ||
        !expect(ActionPluginLoader::pluginRoots().contains(
                  configHome + QLatin1String("/flameshot/plugins")),
                "legacy XDG config root is missing")) {
        return 1;
    }

    QJsonObject manifest = validManifest(QString::fromLocal8Bit(argv[1]));
    if (!expect(writeManifest(path, manifest), "could not write manifest")) {
        return 1;
    }

    ActionPlugin plugin;
    QString error;
    if (!expect(ActionPluginLoader::loadManifest(path, &plugin, &error),
                "valid manifest was rejected") ||
        !expect(plugin.id == QLatin1String("org.flameshot.test-action"),
                "plugin id was not parsed") ||
        !expect(plugin.arguments.size() == 2,
                "plugin arguments were not parsed") ||
        !expect(plugin.outputMode == ActionPlugin::OutputMode::ClipboardText,
                "plugin output mode was not parsed") ||
        !expect(plugin.toolbarOrder == 175,
                "plugin toolbar order was not parsed") ||
        !expect(plugin.shortcut == QLatin1String("Ctrl+Shift+T"),
                "plugin shortcut was not parsed")) {
        std::cerr << error.toStdString() << '\n';
        return 1;
    }

    const QString pluginDirectory =
      directory.filePath("flameshot/plugins/org.flameshot.test-action");
    if (!expect(QDir().mkpath(pluginDirectory),
                "could not create plugin directory") ||
        !expect(writeManifest(pluginDirectory + "/metadata.json", manifest),
                "could not write discoverable manifest")) {
        return 1;
    }
    QStringList discoveryErrors;
    const QList<ActionPlugin> discovered = ActionPluginLoader::discover(
      { directory.filePath("flameshot/plugins") }, &discoveryErrors);
    if (!expect(discovered.size() == 1,
                "plugin in the Flameshot config directory was not found")) {
        std::cerr << discoveryErrors.join('\n').toStdString() << '\n';
        return 1;
    }

    manifest["api_version"] = 2;
    writeManifest(path, manifest);
    if (!expect(!ActionPluginLoader::loadManifest(path, &plugin, &error),
                "unsupported API version was accepted")) {
        return 1;
    }

    manifest = validManifest(QString::fromLocal8Bit(argv[1]));
    QJsonObject command = manifest["command"].toObject();
    command["output"] = "arbitrary-code";
    manifest["command"] = command;
    writeManifest(path, manifest);
    if (!expect(!ActionPluginLoader::loadManifest(path, &plugin, &error),
                "unsupported output mode was accepted")) {
        return 1;
    }

    manifest = validManifest(QString::fromLocal8Bit(argv[1]));
    command = manifest["command"].toObject();
    command["output"] = "json";
    manifest["command"] = command;
    writeManifest(path, manifest);
    if (!expect(ActionPluginLoader::loadManifest(path, &plugin, &error),
                "structured JSON output was rejected") ||
        !expect(plugin.outputMode == ActionPlugin::OutputMode::Json,
                "structured JSON output was not parsed")) {
        return 1;
    }

    manifest = validManifest(QString::fromLocal8Bit(argv[1]));
    QJsonObject toolbar = manifest["toolbar"].toObject();
    toolbar["order"] = -1;
    manifest["toolbar"] = toolbar;
    writeManifest(path, manifest);
    if (!expect(!ActionPluginLoader::loadManifest(path, &plugin, &error),
                "invalid toolbar order was accepted")) {
        return 1;
    }

    return 0;
}
