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

ToolType CopyTool::nameID() const
{
    return ToolType::COPY;
}

QString CopyTool::description() const
{
    return tr("Copy the selection into the clipboard");
}

CaptureTool* CopyTool::copy(QObject* parent)
{
    return new CopyTool(parent);
}

void CopyTool::pressed(const CaptureContext& context)
{
    emit requestAction(REQ_CAPTURE_DONE_OK);
    ScreenshotSaver().saveToClipboard(context.selectedScreenshotArea());
}
