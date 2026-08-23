// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "plugins/actionplugin.h"
#include "tools/capturetool.h"

#include <QList>
#include <QString>
#include <QStringList>

struct ToolDescriptor
{
    enum class Provider
    {
        BuiltIn,
        ExternalAction,
    };

    QString id;
    Provider provider = Provider::BuiltIn;
    CaptureTool::Type legacyType = CaptureTool::NONE;
    int order = 0;
    bool toolbarVisible = true;
    QString shortcut;
    ActionPlugin actionPlugin;

    bool isExternal() const { return provider == Provider::ExternalAction; }
};

class ToolRegistry
{
public:
    ToolRegistry();

    const QList<ToolDescriptor>& tools() const;
    const QStringList& pluginErrors() const;

    static CaptureTool* createTool(const ToolDescriptor& descriptor,
                                   QObject* parent = nullptr);
    static const QList<CaptureTool::Type>& builtInTypes();
    static int builtInPriority(CaptureTool::Type type);
    static QString builtInId(CaptureTool::Type type);

private:
    QList<ToolDescriptor> m_tools;
    QStringList m_pluginErrors;
};
