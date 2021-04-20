// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "undotool.h"
#include <QPainter>

UndoTool::UndoTool(QObject* parent)
  : AbstractActionTool(parent)
{}

bool UndoTool::closeOnButtonPressed() const
{
    return false;
}

QIcon UndoTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor)
    return QIcon(iconPath(background) + "undo-variant.svg");
}
QString UndoTool::name() const
{
    return tr("Undo");
}

ToolType UndoTool::nameID() const
{
    return ToolType::UNDO;
}

QString UndoTool::description() const
{
    return tr("Undo the last modification");
}

CaptureTool* UndoTool::copy(QObject* parent)
{
    return new UndoTool(parent);
}

void UndoTool::pressed(const CaptureContext& context)
{
    Q_UNUSED(context)
    emit requestAction(REQ_UNDO_MODIFICATION);
}
