// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "redotool.h"
#include <QPainter>

RedoTool::RedoTool(QObject* parent)
  : AbstractActionTool(parent)
{}

bool RedoTool::closeOnButtonPressed() const
{
    return false;
}

QIcon RedoTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor)
    return QIcon(iconPath(background) + "redo-variant.svg");
}
QString RedoTool::name() const
{
    return tr("Redo");
}

CaptureTool::Type RedoTool::type() const
{
    return CaptureTool::TYPE_REDO;
}

QString RedoTool::description() const
{
    return tr("Redo the next modification");
}

CaptureTool* RedoTool::copy(QObject* parent)
{
    return new RedoTool(parent);
}

void RedoTool::pressed(CaptureContext& context)
{
    Q_UNUSED(context)
    emit requestAction(REQ_REDO_MODIFICATION);
}
