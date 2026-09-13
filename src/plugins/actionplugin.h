// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QIcon>
#include <QList>
#include <QString>
#include <QStringList>

struct ActionPlugin
{
    enum class OutputMode
    {
        None,
        ClipboardText,
        Json,
    };

    QString id;
    QString name;
    QString description;
    QString executable;
    QStringList arguments;
    QString manifestPath;
    QIcon icon;
    OutputMode outputMode = OutputMode::None;
    int timeoutMs = 30000;
    int toolbarOrder = 1000;
    bool toolbarVisible = true;
    QString shortcut;
};

class ActionPluginLoader
{
public:
    static constexpr int ApiVersion = 1;

    static QString userPluginRoot();
    static QStringList pluginRoots();
    static QList<ActionPlugin> discover(const QStringList& pluginRoots,
                                        QStringList* errors = nullptr);
    static bool loadManifest(const QString& path,
                             ActionPlugin* plugin,
                             QString* error = nullptr);
};
