// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2021 Martin Eckleben & Contributors

#include "sizedecreasetool.h"
#include <QPainter>

SizeDecreaseTool::SizeDecreaseTool(QObject* parent)
  : AbstractActionTool(parent)
{}

bool SizeDecreaseTool::closeOnButtonPressed() const
{
    return false;
}

QIcon SizeDecreaseTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor);
    return QIcon(iconPath(background) + "minus.svg");
}
QString SizeDecreaseTool::name() const
{
    return tr("Decrease Tool Size");
}

ToolType SizeDecreaseTool::nameID() const
{
    return ToolType::SIZEDECREASE;
}

QString SizeDecreaseTool::description() const
{
    return tr("Decrease the size of the other tools");
}

CaptureTool* SizeDecreaseTool::copy(QObject* parent)
{
    return new SizeDecreaseTool(parent);
}

void SizeDecreaseTool::pressed(const CaptureContext& context)
{
    Q_UNUSED(context);
    emit requestAction(REQ_DECREASE_TOOL_SIZE);
}
