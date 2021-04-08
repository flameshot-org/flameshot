// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "movetool.h"
#include <QPainter>

MoveTool::MoveTool(QObject* parent)
  : AbstractActionTool(parent)
{}

bool MoveTool::closeOnButtonPressed() const
{
    return false;
}

QIcon MoveTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor)
    return QIcon(iconPath(background) + "cursor-move.svg");
}
QString MoveTool::name() const
{
    return tr("Move");
}

ToolType MoveTool::nameID() const
{
    return ToolType::MOVE;
}

QString MoveTool::description() const
{
    return tr("Move the selection area");
}

CaptureTool* MoveTool::copy(QObject* parent)
{
    return new MoveTool(parent);
}

void MoveTool::pressed(const CaptureContext& context)
{
    Q_UNUSED(context)
    emit requestAction(REQ_MOVE_MODE);
}

bool MoveTool::isSelectable() const
{
    return true;
}
