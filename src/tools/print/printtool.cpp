// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Ouyang Chunhui & Contributor

#include "printtool.h"
#include "src/utils/screenshotsaver.h"
#include <QPainter>

PrintTool::PrintTool(QObject* parent)
  : AbstractActionTool(parent)
{}

bool PrintTool::closeOnButtonPressed() const
{
    return true;
}

QIcon PrintTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor)
    return QIcon(iconPath(background) + "content-print.svg");
}
QString PrintTool::name() const
{
    return tr("Print");
}

CaptureTool::Type PrintTool::type() const
{
    return CaptureTool::TYPE_PRINT;
}

QString PrintTool::description() const
{
    return tr("Print");
}

CaptureTool* PrintTool::copy(QObject* parent)
{
    return new PrintTool(parent);
}

void PrintTool::pressed(CaptureContext& context)
{
    emit requestAction(REQ_CLEAR_SELECTION);
    context.request.addTask(CaptureRequest::PRINT_SYSTEM);
    emit requestAction(REQ_CAPTURE_DONE_OK);
    emit requestAction(REQ_CLOSE_GUI);
}
