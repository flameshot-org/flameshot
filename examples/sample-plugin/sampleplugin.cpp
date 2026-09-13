// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#include "sampleplugin.h"

#include <QPainter>

SampleTool::SampleTool(QObject* parent)
  : AbstractActionTool(parent)
{}

QIcon SampleTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(background);
    Q_UNUSED(inEditor);
    return QIcon(QStringLiteral(":/img/material/black/invert.svg"));
}

QString SampleTool::name() const
{
    return QStringLiteral("Sample Plugin Tool");
}

CaptureTool::Type SampleTool::type() const
{
    return CaptureTool::NONE;
}

QString SampleTool::description() const
{
    return QStringLiteral(
      "Sample plugin tool demonstrating dynamic extensibility.");
}

CaptureTool* SampleTool::copy(QObject* parent)
{
    return new SampleTool(parent);
}

void SampleTool::process(QPainter& painter, const QPixmap& pixmap)
{
    Q_UNUSED(painter);
    Q_UNUSED(pixmap);
}

void SampleTool::pressed(CaptureContext& context)
{
    Q_UNUSED(context);
}

bool SampleTool::closeOnButtonPressed() const
{
    return false;
}

SamplePlugin::SamplePlugin(QObject* parent)
  : QObject(parent)
{}

QString SamplePlugin::pluginId() const
{
    return QStringLiteral("org.flameshot.sample");
}

QString SamplePlugin::pluginName() const
{
    return QStringLiteral("Sample Extensibility Plugin");
}

QString SamplePlugin::pluginVersion() const
{
    return QStringLiteral("1.0.0");
}

QString SamplePlugin::pluginAuthor() const
{
    return QStringLiteral("Flameshot Contributors");
}

QString SamplePlugin::pluginDescription() const
{
    return QStringLiteral("A reference plugin demonstrating how to create "
                          "custom tools for Flameshot.");
}

QIcon SamplePlugin::pluginIcon() const
{
    return QIcon(QStringLiteral(":/img/material/black/plugin.svg"));
}

QList<CaptureTool*> SamplePlugin::createTools(QObject* parent)
{
    return { new SampleTool(parent) };
}
