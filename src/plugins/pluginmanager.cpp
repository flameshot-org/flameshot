// SPDX-License-Identifier: GPL-3.0-or-later

#include "pluginmanager.h"

#include "utils/confighandler.h"

#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTextStream>

namespace {
constexpr qint64 MaxPackageSize = 32 * 1024 * 1024;
constexpr qint64 MaxInstalledSize = 64 * 1024 * 1024;
constexpr int MaxFileCount = 1000;

PluginManagerResult failure(const QString& message)
{
    return { false, message };
}

PluginManagerResult success(const QString& message)
{
    return { true, message };
}

bool validateRelativePath(const QString& path)
{
    if (path.isEmpty() || QDir::isAbsolutePath(path) ||
        path.contains(QLatin1Char('\0')) || path.contains(QLatin1Char('\n')) ||
        path.contains(QLatin1Char('\r'))) {
        return false;
    }
    const QString clean = QDir::cleanPath(path);
    return clean != QLatin1String("..") &&
           !clean.startsWith(QLatin1String("../"));
}

bool inspectTree(const QString& root, QString* error)
{
    QDir rootDirectory(root);
    QDirIterator iterator(root,
                          QDir::AllEntries | QDir::NoDotAndDotDot |
                            QDir::Hidden | QDir::System,
                          QDirIterator::Subdirectories);
    qint64 totalSize = 0;
    int fileCount = 0;
    while (iterator.hasNext()) {
        const QString path = iterator.next();
        const QFileInfo info(path);
        const QString relative = rootDirectory.relativeFilePath(path);
        if (!validateRelativePath(relative) || info.isSymLink()) {
            *error = QCoreApplication::translate("PluginManager",
                                                 "Unsafe package entry: %1")
                       .arg(relative);
            return false;
        }
        if (!info.isDir() && !info.isFile()) {
            *error = QCoreApplication::translate(
                       "PluginManager", "Unsupported package entry: %1")
                       .arg(relative);
            return false;
        }
        if (info.isFile()) {
            totalSize += info.size();
            ++fileCount;
            if (totalSize > MaxInstalledSize || fileCount > MaxFileCount) {
                *error = QCoreApplication::translate(
                  "PluginManager", "Plugin package exceeds the safety limits.");
                return false;
            }
        }
    }
    return true;
}

bool copyTree(const QString& source, const QString& target, QString* error)
{
    if (!inspectTree(source, error)) {
        return false;
    }

    QDir sourceDirectory(source);
    QDirIterator iterator(source,
                          QDir::AllEntries | QDir::NoDotAndDotDot |
                            QDir::Hidden | QDir::System,
                          QDirIterator::Subdirectories);
    while (iterator.hasNext()) {
        const QString sourcePath = iterator.next();
        const QFileInfo info(sourcePath);
        const QString relative = sourceDirectory.relativeFilePath(sourcePath);
        const QString targetPath = QDir(target).filePath(relative);
        if (info.isDir()) {
            if (!QDir().mkpath(targetPath)) {
                *error = QCoreApplication::translate("PluginManager",
                                                     "Could not create %1")
                           .arg(targetPath);
                return false;
            }
        } else {
            if (!QDir().mkpath(QFileInfo(targetPath).absolutePath()) ||
                !QFile::copy(sourcePath, targetPath)) {
                *error = QCoreApplication::translate("PluginManager",
                                                     "Could not copy %1")
                           .arg(relative);
                return false;
            }
            QFile::setPermissions(targetPath, info.permissions());
        }
    }
    return true;
}

bool runTar(const QStringList& arguments,
            QByteArray* standardOutput,
            QString* error)
{
    const QString tar = QStandardPaths::findExecutable(QStringLiteral("tar"));
    if (tar.isEmpty()) {
        *error = QCoreApplication::translate(
          "PluginManager", "The tar command is required to install packages.");
        return false;
    }

    QProcess process;
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    environment.insert(QStringLiteral("LC_ALL"), QStringLiteral("C"));
    process.setProcessEnvironment(environment);
    process.start(tar, arguments);
    if (!process.waitForFinished(30000) ||
        process.exitStatus() != QProcess::NormalExit ||
        process.exitCode() != 0) {
        *error = QString::fromUtf8(process.readAllStandardError()).trimmed();
        if (error->isEmpty()) {
            *error = QCoreApplication::translate(
              "PluginManager", "Could not read the plugin package.");
        }
        return false;
    }
    if (standardOutput) {
        *standardOutput = process.readAllStandardOutput();
    }
    return true;
}

bool extractPackage(const QString& packagePath,
                    const QString& target,
                    QString* error)
{
    const QFileInfo packageInfo(packagePath);
    if (!packageInfo.isFile() || packageInfo.size() > MaxPackageSize) {
        *error = QCoreApplication::translate(
          "PluginManager", "Plugin package is missing or too large.");
        return false;
    }

    QByteArray listing;
    if (!runTar({ QStringLiteral("--list"),
                  QStringLiteral("--file"),
                  packageInfo.absoluteFilePath() },
                &listing,
                error)) {
        return false;
    }
    const QList<QByteArray> entries = listing.split('\n');
    int entryCount = 0;
    for (const QByteArray& rawEntry : entries) {
        if (rawEntry.isEmpty()) {
            continue;
        }
        ++entryCount;
        const QString entry = QString::fromUtf8(rawEntry);
        if (entryCount > MaxFileCount || !validateRelativePath(entry)) {
            *error = QCoreApplication::translate("PluginManager",
                                                 "Unsafe package entry: %1")
                       .arg(entry);
            return false;
        }
    }

    QByteArray verboseListing;
    if (!runTar({ QStringLiteral("--list"),
                  QStringLiteral("--verbose"),
                  QStringLiteral("--file"),
                  packageInfo.absoluteFilePath() },
                &verboseListing,
                error)) {
        return false;
    }
    for (const QByteArray& line : verboseListing.split('\n')) {
        if (!line.isEmpty() && line.at(0) != '-' && line.at(0) != 'd') {
            *error =
              QCoreApplication::translate("PluginManager",
                                          "Plugin packages may contain only "
                                          "regular files and directories.");
            return false;
        }
    }

    if (!runTar({ QStringLiteral("--extract"),
                  QStringLiteral("--file"),
                  packageInfo.absoluteFilePath(),
                  QStringLiteral("--directory"),
                  target,
                  QStringLiteral("--no-same-owner"),
                  QStringLiteral("--no-same-permissions") },
                nullptr,
                error)) {
        return false;
    }
    return inspectTree(target, error);
}

ActionPlugin findPlugin(const QString& pluginId, bool* found)
{
    const QList<ActionPlugin> plugins =
      ActionPluginLoader::discover(ActionPluginLoader::pluginRoots());
    for (const ActionPlugin& plugin : plugins) {
        if (plugin.id == pluginId) {
            *found = true;
            return plugin;
        }
    }
    *found = false;
    return {};
}

void printPluginHelp(QTextStream& output)
{
    output
      << "Usage:\n"
         "  flameshot plugins list\n"
         "  flameshot plugins install <directory|package.flameshot-plugin>\n"
         "  flameshot plugins enable <plugin-id>\n"
         "  flameshot plugins disable <plugin-id>\n"
         "  flameshot plugins remove <plugin-id>\n"
         "  flameshot plugins doctor\n"
         "  flameshot plugins roots\n";
}
} // namespace

PluginManagerResult PluginManager::install(const QString& sourcePath)
{
    const QFileInfo sourceInfo(sourcePath);
    if (!sourceInfo.exists()) {
        return failure(QCoreApplication::translate("PluginManager",
                                                   "Source does not exist: %1")
                         .arg(sourcePath));
    }

    const QString root = ActionPluginLoader::userPluginRoot();
    if (!QDir().mkpath(root)) {
        return failure(QCoreApplication::translate(
                         "PluginManager", "Could not create plugin root: %1")
                         .arg(root));
    }

    QTemporaryDir staging(
      QDir(root).filePath(QStringLiteral(".install-XXXXXX")));
    if (!staging.isValid()) {
        return failure(QCoreApplication::translate(
          "PluginManager", "Could not create a temporary install directory."));
    }

    QString error;
    const bool prepared =
      sourceInfo.isDir()
        ? copyTree(sourceInfo.absoluteFilePath(), staging.path(), &error)
        : extractPackage(sourceInfo.absoluteFilePath(), staging.path(), &error);
    if (!prepared) {
        return failure(error);
    }

    ActionPlugin plugin;
    if (!ActionPluginLoader::loadManifest(
          QDir(staging.path()).filePath(QStringLiteral("metadata.json")),
          &plugin,
          &error)) {
        return failure(QCoreApplication::translate(
                         "PluginManager", "Invalid plugin manifest: %1")
                         .arg(error));
    }

    const QString target = QDir(root).filePath(plugin.id);
    if (QFileInfo::exists(target)) {
        return failure(
          QCoreApplication::translate(
            "PluginManager", "Plugin %1 is already installed for this user.")
            .arg(plugin.id));
    }
    staging.setAutoRemove(false);
    if (!QDir().rename(staging.path(), target)) {
        staging.setAutoRemove(true);
        return failure(QCoreApplication::translate(
                         "PluginManager", "Could not activate plugin %1.")
                         .arg(plugin.id));
    }
    ConfigHandler().setPluginEnabled(plugin.id, plugin.toolbarVisible);
    return success(
      QCoreApplication::translate("PluginManager", "Installed plugin %1 in %2")
        .arg(plugin.id, target));
}

PluginManagerResult PluginManager::remove(const QString& pluginId)
{
    if (!validPluginId(pluginId)) {
        return failure(
          QCoreApplication::translate("PluginManager", "Invalid plugin id."));
    }
    const QString target =
      QDir(ActionPluginLoader::userPluginRoot()).filePath(pluginId);
    const QFileInfo info(target);
    if (!info.isDir() || info.isSymLink()) {
        return failure(QCoreApplication::translate(
          "PluginManager", "Plugin is not installed in the user plugin root."));
    }
    if (!QDir(target).removeRecursively()) {
        return failure(QCoreApplication::translate(
                         "PluginManager", "Could not remove plugin %1.")
                         .arg(pluginId));
    }
    return success(
      QCoreApplication::translate("PluginManager", "Removed plugin %1.")
        .arg(pluginId));
}

PluginManagerResult PluginManager::setEnabled(const QString& pluginId,
                                              bool enabled)
{
    bool found = false;
    findPlugin(pluginId, &found);
    if (!found) {
        return failure(QCoreApplication::translate("PluginManager",
                                                   "Plugin %1 was not found.")
                         .arg(pluginId));
    }
    ConfigHandler().setPluginEnabled(pluginId, enabled);
    return success(
      QCoreApplication::translate("PluginManager", "Plugin %1 is now %2.")
        .arg(pluginId,
             enabled ? QStringLiteral("enabled") : QStringLiteral("disabled")));
}

QList<ActionPlugin> PluginManager::list(QStringList* errors)
{
    return ActionPluginLoader::discover(ActionPluginLoader::pluginRoots(),
                                        errors);
}

QStringList PluginManager::doctor(bool* healthy)
{
    QStringList messages;
    QStringList errors;
    const QList<ActionPlugin> plugins = list(&errors);
    for (const QString& root : ActionPluginLoader::pluginRoots()) {
        messages.append(
          QCoreApplication::translate("PluginManager", "Search root: %1%2")
            .arg(root,
                 QDir(root).exists() ? QStringLiteral(" [present]")
                                     : QStringLiteral(" [missing]")));
    }
    for (const ActionPlugin& plugin : plugins) {
        messages.append(
          QCoreApplication::translate("PluginManager", "OK: %1 -> %2")
            .arg(plugin.id, plugin.executable));
    }
    for (const QString& error : errors) {
        messages.append(
          QCoreApplication::translate("PluginManager", "ERROR: %1").arg(error));
    }
    if (healthy) {
        *healthy = errors.isEmpty();
    }
    return messages;
}

bool PluginManager::validPluginId(const QString& pluginId)
{
    static const QRegularExpression expression(
      QStringLiteral("^[a-z0-9]+(?:[._-][a-z0-9]+)+$"));
    return expression.match(pluginId).hasMatch();
}

int PluginManagerCli::run(const QStringList& arguments)
{
    QTextStream output(stdout);
    QTextStream errorOutput(stderr);
    if (arguments.isEmpty() || arguments.first() == QLatin1String("help") ||
        arguments.first() == QLatin1String("--help") ||
        arguments.first() == QLatin1String("-h")) {
        printPluginHelp(output);
        return 0;
    }

    const QString command = arguments.first();
    if (command == QLatin1String("list")) {
        QStringList errors;
        const QList<ActionPlugin> plugins = PluginManager::list(&errors);
        ConfigHandler config;
        for (const ActionPlugin& plugin : plugins) {
            output << (config.pluginEnabled(plugin.id, plugin.toolbarVisible)
                         ? "enabled  "
                         : "disabled ")
                   << plugin.id << "\t" << plugin.name << "\n";
        }
        for (const QString& message : errors) {
            errorOutput << "error: " << message << "\n";
        }
        return errors.isEmpty() ? 0 : 1;
    }
    if (command == QLatin1String("doctor")) {
        bool healthy = false;
        for (const QString& message : PluginManager::doctor(&healthy)) {
            output << message << "\n";
        }
        return healthy ? 0 : 1;
    }
    if (command == QLatin1String("roots")) {
        for (const QString& root : ActionPluginLoader::pluginRoots()) {
            output << root << "\n";
        }
        return 0;
    }

    if (arguments.size() != 2) {
        printPluginHelp(errorOutput);
        return 2;
    }
    PluginManagerResult result;
    if (command == QLatin1String("install")) {
        result = PluginManager::install(arguments.at(1));
    } else if (command == QLatin1String("enable")) {
        result = PluginManager::setEnabled(arguments.at(1), true);
    } else if (command == QLatin1String("disable")) {
        result = PluginManager::setEnabled(arguments.at(1), false);
    } else if (command == QLatin1String("remove")) {
        result = PluginManager::remove(arguments.at(1));
    } else {
        errorOutput << "Unknown plugins command: " << command << "\n";
        printPluginHelp(errorOutput);
        return 2;
    }

    (result.success ? output : errorOutput) << result.message << "\n";
    return result.success ? 0 : 1;
}
