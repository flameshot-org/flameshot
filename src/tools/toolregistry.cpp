// SPDX-License-Identifier: GPL-3.0-or-later

#include "toolregistry.h"

#include "tools/plugin/plugintool.h"
#include "tools/toolfactory.h"
#include "utils/confighandler.h"

#include <QSet>

#include <algorithm>

ToolRegistry::ToolRegistry()
{
    QSet<QString> knownIds;
    for (const CaptureTool::Type type : builtInTypes()) {
        ToolDescriptor descriptor;
        descriptor.id = builtInId(type);
        descriptor.legacyType = type;
        descriptor.order = builtInPriority(type) * 10;
        m_tools.append(descriptor);
        knownIds.insert(descriptor.id);
    }

    const QList<ActionPlugin> plugins = ActionPluginLoader::discover(
      ActionPluginLoader::pluginRoots(), &m_pluginErrors);
    ConfigHandler config;
    for (const ActionPlugin& plugin : plugins) {
        if (knownIds.contains(plugin.id)) {
            m_pluginErrors.append(
              QStringLiteral("%1: plugin id collides with a built-in tool")
                .arg(plugin.id));
            continue;
        }
        knownIds.insert(plugin.id);
        ToolDescriptor descriptor;
        descriptor.id = plugin.id;
        descriptor.provider = ToolDescriptor::Provider::ExternalAction;
        descriptor.legacyType = CaptureTool::TYPE_PLUGIN;
        descriptor.order = plugin.toolbarOrder;
        descriptor.toolbarVisible =
          config.pluginEnabled(plugin.id, plugin.toolbarVisible);
        descriptor.shortcut = config.pluginShortcut(plugin.id, plugin.shortcut);
        descriptor.actionPlugin = plugin;
        m_tools.append(descriptor);
    }

    std::stable_sort(
      m_tools.begin(),
      m_tools.end(),
      [](const ToolDescriptor& left, const ToolDescriptor& right) {
          if (left.order != right.order) {
              return left.order < right.order;
          }
          return left.id < right.id;
      });
}

const QList<ToolDescriptor>& ToolRegistry::tools() const
{
    return m_tools;
}

const QStringList& ToolRegistry::pluginErrors() const
{
    return m_pluginErrors;
}

CaptureTool* ToolRegistry::createTool(const ToolDescriptor& descriptor,
                                      QObject* parent)
{
    if (descriptor.provider == ToolDescriptor::Provider::ExternalAction) {
        return new PluginActionTool(descriptor.actionPlugin, parent);
    }
    return ToolFactory().CreateTool(descriptor.legacyType, parent);
}

const QList<CaptureTool::Type>& ToolRegistry::builtInTypes()
{
    static const QList<CaptureTool::Type> types = {
        CaptureTool::TYPE_PENCIL,        CaptureTool::TYPE_DRAWER,
        CaptureTool::TYPE_ARROW,         CaptureTool::TYPE_SELECTION,
        CaptureTool::TYPE_RECTANGLE,     CaptureTool::TYPE_CIRCLE,
        CaptureTool::TYPE_MARKER,        CaptureTool::TYPE_TEXT,
        CaptureTool::TYPE_CIRCLECOUNT,   CaptureTool::TYPE_PIXELATE,
        CaptureTool::TYPE_INVERT,        CaptureTool::TYPE_MOVESELECTION,
        CaptureTool::TYPE_UNDO,          CaptureTool::TYPE_REDO,
        CaptureTool::TYPE_COPY,          CaptureTool::TYPE_SAVE,
        CaptureTool::TYPE_EXIT,
#ifdef ENABLE_IMGUR
        CaptureTool::TYPE_IMAGEUPLOADER,
#endif
#if !defined(Q_OS_MACOS)
        CaptureTool::TYPE_OPEN_APP,
#endif
        CaptureTool::TYPE_PIN,           CaptureTool::TYPE_SIZEINCREASE,
        CaptureTool::TYPE_SIZEDECREASE,  CaptureTool::TYPE_ACCEPT,
    };
    return types;
}

int ToolRegistry::builtInPriority(CaptureTool::Type type)
{
    switch (type) {
        case CaptureTool::TYPE_PENCIL:
            return 0;
        case CaptureTool::TYPE_DRAWER:
            return 1;
        case CaptureTool::TYPE_ARROW:
            return 2;
        case CaptureTool::TYPE_SELECTION:
            return 3;
        case CaptureTool::TYPE_RECTANGLE:
            return 4;
        case CaptureTool::TYPE_CIRCLE:
            return 5;
        case CaptureTool::TYPE_MARKER:
            return 6;
        case CaptureTool::TYPE_TEXT:
            return 7;
        case CaptureTool::TYPE_PIXELATE:
            return 8;
        case CaptureTool::TYPE_INVERT:
            return 9;
        case CaptureTool::TYPE_CIRCLECOUNT:
            return 10;
        case CaptureTool::TYPE_MOVESELECTION:
            return 12;
        case CaptureTool::TYPE_UNDO:
            return 13;
        case CaptureTool::TYPE_REDO:
            return 14;
        case CaptureTool::TYPE_COPY:
            return 15;
        case CaptureTool::TYPE_SAVE:
            return 16;
#ifdef ENABLE_IMGUR
        case CaptureTool::TYPE_IMAGEUPLOADER:
            return 17;
#endif
        case CaptureTool::TYPE_ACCEPT:
            return 18;
#if !defined(Q_OS_MACOS)
        case CaptureTool::TYPE_OPEN_APP:
            return 19;
#endif
        case CaptureTool::TYPE_EXIT:
            return 20;
        case CaptureTool::TYPE_PIN:
            return 21;
        case CaptureTool::TYPE_SIZEINCREASE:
            return 22;
        case CaptureTool::TYPE_SIZEDECREASE:
            return 23;
        default:
            return 1000;
    }
}

QString ToolRegistry::builtInId(CaptureTool::Type type)
{
    switch (type) {
        case CaptureTool::TYPE_PENCIL:
            return QStringLiteral("org.flameshot.pencil");
        case CaptureTool::TYPE_DRAWER:
            return QStringLiteral("org.flameshot.line");
        case CaptureTool::TYPE_ARROW:
            return QStringLiteral("org.flameshot.arrow");
        case CaptureTool::TYPE_SELECTION:
            return QStringLiteral("org.flameshot.selection");
        case CaptureTool::TYPE_RECTANGLE:
            return QStringLiteral("org.flameshot.rectangle");
        case CaptureTool::TYPE_CIRCLE:
            return QStringLiteral("org.flameshot.circle");
        case CaptureTool::TYPE_MARKER:
            return QStringLiteral("org.flameshot.marker");
        case CaptureTool::TYPE_MOVESELECTION:
            return QStringLiteral("org.flameshot.move-selection");
        case CaptureTool::TYPE_UNDO:
            return QStringLiteral("org.flameshot.undo");
        case CaptureTool::TYPE_COPY:
            return QStringLiteral("org.flameshot.copy");
        case CaptureTool::TYPE_SAVE:
            return QStringLiteral("org.flameshot.save");
        case CaptureTool::TYPE_EXIT:
            return QStringLiteral("org.flameshot.exit");
#ifdef ENABLE_IMGUR
        case CaptureTool::TYPE_IMAGEUPLOADER:
            return QStringLiteral("org.flameshot.imgur-upload");
#endif
#if !defined(Q_OS_MACOS)
        case CaptureTool::TYPE_OPEN_APP:
            return QStringLiteral("org.flameshot.open-app");
#endif
        case CaptureTool::TYPE_PIXELATE:
            return QStringLiteral("org.flameshot.pixelate");
        case CaptureTool::TYPE_REDO:
            return QStringLiteral("org.flameshot.redo");
        case CaptureTool::TYPE_PIN:
            return QStringLiteral("org.flameshot.pin");
        case CaptureTool::TYPE_TEXT:
            return QStringLiteral("org.flameshot.text");
        case CaptureTool::TYPE_CIRCLECOUNT:
            return QStringLiteral("org.flameshot.circle-counter");
        case CaptureTool::TYPE_SIZEINCREASE:
            return QStringLiteral("org.flameshot.size-increase");
        case CaptureTool::TYPE_SIZEDECREASE:
            return QStringLiteral("org.flameshot.size-decrease");
        case CaptureTool::TYPE_INVERT:
            return QStringLiteral("org.flameshot.invert");
        case CaptureTool::TYPE_ACCEPT:
            return QStringLiteral("org.flameshot.accept");
        default:
            return {};
    }
}
