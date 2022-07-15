// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "copytool.h"
#include "src/utils/screenshotsaver.h"
#include <QPainter>

CopyTool::CopyTool(QObject* parent)
  : AbstractActionTool(parent)
{}

bool CopyTool::closeOnButtonPressed() const
{
    return true;
}

QIcon CopyTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor)
    return QIcon(iconPath(background) + "content-copy.svg");
}
QString CopyTool::name() const
{
    return tr("Copy");
}

CaptureTool::Type CopyTool::type() const
{
    return CaptureTool::TYPE_COPY;
}

QString CopyTool::description() const
{
    return tr("Copy selection to clipboard");
}

CaptureTool* CopyTool::copy(QObject* parent)
{
    return new CopyTool(parent);
}

void CopyTool::pressed(CaptureContext& context)
{
    emit requestAction(REQ_CLEAR_SELECTION);
    context.request.addTask(CaptureRequest::COPY);
    emit requestAction(REQ_CAPTURE_DONE_OK);
    emit requestAction(REQ_CLOSE_GUI);
}
