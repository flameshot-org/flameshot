// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "accepttool.h"
#include "src/utils/screenshotsaver.h"
#include <QApplication>
#include <QPainter>
#include <QStyle>
#if defined(Q_OS_MACOS)
#include "src/widgets/capture/capturewidget.h"
#include <QWidget>
#endif

AcceptTool::AcceptTool(QObject* parent)
  : AbstractActionTool(parent)
{}

bool AcceptTool::closeOnButtonPressed() const
{
    return true;
}

QIcon AcceptTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor)
    return QIcon(iconPath(background) + "accept.svg");
}

QString AcceptTool::name() const
{
    return tr("Accept");
}

CaptureTool::Type AcceptTool::type() const
{
    return CaptureTool::TYPE_ACCEPT;
}

QString AcceptTool::description() const
{
    return tr("Accept the capture");
}

CaptureTool* AcceptTool::copy(QObject* parent)
{
    return new AcceptTool(parent);
}

void AcceptTool::pressed(CaptureContext& context)
{
    emit requestAction(REQ_CAPTURE_DONE_OK);
    if (context.request.tasks() & CaptureRequest::PIN) {
        QRect geometry = context.selection;
        geometry.moveTopLeft(geometry.topLeft() + context.widgetOffset);
        context.request.addTask(CaptureRequest::PIN);
    }
    emit requestAction(REQ_CLOSE_GUI);
}
