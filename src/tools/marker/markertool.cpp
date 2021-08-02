// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "markertool.h"
#include <QPainter>

#define PADDING_VALUE 14

MarkerTool::MarkerTool(QObject* parent)
  : AbstractTwoPointTool(parent)
{
    m_supportsOrthogonalAdj = true;
    m_supportsDiagonalAdj = true;
}

QIcon MarkerTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor)
    return QIcon(iconPath(background) + "marker.svg");
}
QString MarkerTool::name() const
{
    return tr("Marker");
}

CaptureTool::Type MarkerTool::type() const
{
    return CaptureTool::TYPE_MARKER;
}

QString MarkerTool::description() const
{
    return tr("Set the Marker as the paint tool");
}

QRect MarkerTool::mousePreviewRect(const CaptureContext& context) const
{
    int width = PADDING_VALUE + context.toolSize;
    QRect rect(0, 0, width + 2, width + 2);
    rect.moveCenter(context.mousePos);
    return rect;
}

CaptureTool* MarkerTool::copy(QObject* parent)
{
    auto* tool = new MarkerTool(parent);
    copyParams(this, tool);
    return tool;
}

void MarkerTool::process(QPainter& painter, const QPixmap& pixmap)
{
    Q_UNUSED(pixmap)
    auto compositionMode = painter.compositionMode();
    qreal opacity = painter.opacity();
    auto pen = painter.pen();
    painter.setCompositionMode(QPainter::CompositionMode_Multiply);
    painter.setOpacity(0.35);
    painter.setPen(QPen(color(), size()));
    painter.drawLine(points().first, points().second);
    painter.setPen(pen);
    painter.setOpacity(opacity);
    painter.setCompositionMode(compositionMode);
}

void MarkerTool::paintMousePreview(QPainter& painter,
                                   const CaptureContext& context)
{
    auto compositionMode = painter.compositionMode();
    qreal opacity = painter.opacity();
    auto pen = painter.pen();
    painter.setCompositionMode(QPainter::CompositionMode_Multiply);
    painter.setOpacity(0.35);
    painter.setPen(QPen(context.color, PADDING_VALUE + context.toolSize));
    painter.drawLine(context.mousePos, context.mousePos);
    painter.setPen(pen);
    painter.setOpacity(opacity);
    painter.setCompositionMode(compositionMode);
}

void MarkerTool::drawStart(const CaptureContext& context)
{
    AbstractTwoPointTool::drawStart(context);
    onSizeChanged(context.toolSize + PADDING_VALUE);
}

void MarkerTool::pressed(CaptureContext& context)
{
    Q_UNUSED(context)
}
