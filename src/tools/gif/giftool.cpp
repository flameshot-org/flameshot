// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2026 Alejandro Sirgo Rica & Contributors

#include "giftool.h"

#include <QPainter>

GifTool::GifTool(QObject* parent)
  : AbstractActionTool(parent)
{}

bool GifTool::closeOnButtonPressed() const
{
    return true;
}

QIcon GifTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor)
    return QIcon(iconPath(background) + "gif.svg");
}

QString GifTool::name() const
{
    return tr("GIF");
}

CaptureTool::Type GifTool::type() const
{
    return CaptureTool::TYPE_GIF;
}

QString GifTool::description() const
{
    return tr("Record the selection as an animated GIF");
}

CaptureTool* GifTool::copy(QObject* parent)
{
    return new GifTool(parent);
}

void GifTool::pressed(CaptureContext& context)
{
    QRect selection = context.selection;
    if (selection.isNull() || selection.isEmpty()) {
        selection = context.screenshot.rect();
    }

    // The context selection is expressed in screenshot pixels, while screen
    // coordinates are logical pixels. Convert through the device pixel ratio.
    const qreal dpr = qMax(1.0, context.screenshot.devicePixelRatio());
    const QRect region(
      context.widgetOffset.x() + qRound(selection.x() / dpr),
      context.widgetOffset.y() + qRound(selection.y() / dpr),
      qMax(1, qRound(selection.width() / dpr)),
      qMax(1, qRound(selection.height() / dpr)));

    context.request.setGifRegion(region);
    context.request.addTask(CaptureRequest::GIF);
    emit requestAction(REQ_CAPTURE_DONE_OK);
    emit requestAction(REQ_CLOSE_GUI);
}
