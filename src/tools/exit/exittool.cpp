// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "exittool.h"
#include <QPainter>

ExitTool::ExitTool(QObject* parent)
  : AbstractActionTool(parent)
{}

bool ExitTool::closeOnButtonPressed() const
{
    return true;
}

QIcon ExitTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor)
    return QIcon(iconPath(background) + "close.svg");
}
QString ExitTool::name() const
{
    return tr("Exit");
}

ToolType ExitTool::nameID() const
{
    return ToolType::EXIT;
}

QString ExitTool::description() const
{
    return tr("Leave the capture screen");
}

CaptureTool* ExitTool::copy(QObject* parent)
{
    return new ExitTool(parent);
}

void ExitTool::pressed(const CaptureContext& context)
{
    Q_UNUSED(context)
    emit requestAction(REQ_CLOSE_GUI);
}
