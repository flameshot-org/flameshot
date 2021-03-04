// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2021 Martin Eckleben & Contributors

#include "sizeincreasetool.h"
#include <QPainter>

SizeIncreaseTool::SizeIncreaseTool(QObject* parent)
  : AbstractActionTool(parent)
{}

bool SizeIncreaseTool::closeOnButtonPressed() const
{
    return false;
}

QIcon SizeIncreaseTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor);
    return QIcon(iconPath(background) + "plus.svg");
}
QString SizeIncreaseTool::name() const
{
    return tr("Increase Tool Size");
}

ToolType SizeIncreaseTool::nameID() const
{
    return ToolType::SIZEINCREASE;
}

QString SizeIncreaseTool::description() const
{
    return tr("Increase the size of the other tools");
}

CaptureTool* SizeIncreaseTool::copy(QObject* parent)
{
    return new SizeIncreaseTool(parent);
}

void SizeIncreaseTool::pressed(const CaptureContext& context)
{
    Q_UNUSED(context);
    emit requestAction(REQ_INCREASE_TOOL_SIZE);
}
