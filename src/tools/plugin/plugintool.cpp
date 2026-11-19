// SPDX-License-Identifier: GPL-3.0-or-later

#include "plugintool.h"
#include "plugins/actionpluginrunner.h"

PluginActionTool::PluginActionTool(const ActionPlugin& plugin, QObject* parent)
  : AbstractActionTool(parent)
  , m_plugin(plugin)
{}

bool PluginActionTool::closeOnButtonPressed() const
{
    return true;
}

QIcon PluginActionTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor)
    if (!m_plugin.icon.isNull()) {
        return m_plugin.icon;
    }
    return QIcon(iconPath(background) + "apps.svg");
}

QString PluginActionTool::name() const
{
    return m_plugin.name;
}

CaptureTool::Type PluginActionTool::type() const
{
    return CaptureTool::TYPE_PLUGIN;
}

QString PluginActionTool::description() const
{
    return m_plugin.description;
}

CaptureTool* PluginActionTool::copy(QObject* parent)
{
    return new PluginActionTool(m_plugin, parent);
}

void PluginActionTool::pressed(CaptureContext& context)
{
    ActionPluginRunner* runner =
      ActionPluginRunner::run(m_plugin, context.selectedScreenshotArea());
    connect(runner, &ActionPluginRunner::finished, this, [this](bool) {
        emit requestAction(REQ_CLOSE_GUI);
    });
    emit requestAction(REQ_CAPTURE_DONE_OK);
    emit requestAction(REQ_HIDE_GUI);
}
