// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "pintool.h"
#include "src/core/qguiappcurrentscreen.h"
#include "src/tools/pin/pinwidget.h"
#include <QScreen>

PinTool::PinTool(QObject* parent)
  : AbstractActionTool(parent)
{}

bool PinTool::closeOnButtonPressed() const
{
    return true;
}

QIcon PinTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor)
    return QIcon(iconPath(background) + "pin.svg");
}
QString PinTool::name() const
{
    return tr("Pin Tool");
}

CaptureTool::Type PinTool::type() const
{
    return CaptureTool::TYPE_PIN;
}

QString PinTool::description() const
{
    return tr("Pin image on the desktop");
}

CaptureTool* PinTool::copy(QObject* parent)
{
    return new PinTool(parent);
}

void PinTool::pressed(CaptureContext& context)
{
    emit requestAction(REQ_CAPTURE_DONE_OK);
    QRect geometry = context.selection;
    geometry.setTopLeft(geometry.topLeft() + context.widgetOffset);
    context.request()->addPinTask(geometry);
    emit requestAction(REQ_CLOSE_GUI);
}
