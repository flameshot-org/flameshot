// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "actionplugin.h"

#include <QString>
#include <QStringList>

struct PluginManagerResult
{
    bool success = false;
    QString message;
};

class PluginManager
{
public:
    static PluginManagerResult install(const QString& sourcePath);
    static PluginManagerResult remove(const QString& pluginId);
    static PluginManagerResult setEnabled(const QString& pluginId,
                                          bool enabled);
    static QList<ActionPlugin> list(QStringList* errors = nullptr);
    static QStringList doctor(bool* healthy = nullptr);

private:
    static bool validPluginId(const QString& pluginId);
};

class PluginManagerCli
{
public:
    static int run(const QStringList& arguments);
};
