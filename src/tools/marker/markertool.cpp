// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "markertool.h"
#include <QPainter>

namespace {

#define PADDING_VALUE 14

}

MarkerTool::MarkerTool(QObject* parent)
  : AbstractTwoPointTool(parent)
{
    m_supportsOrthogonalAdj = true;
    m_supportsDiagonalAdj = true;
}

QIcon MarkerTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor);
    return QIcon(iconPath(background) + "marker.svg");
}
QString MarkerTool::name() const
{
    return tr("Marker");
}

ToolType MarkerTool::nameID() const
{
    return ToolType::MARKER;
}

QString MarkerTool::description() const
{
    return tr("Set the Marker as the paint tool");
}

CaptureTool* MarkerTool::copy(QObject* parent)
{
    MarkerTool* tool = new MarkerTool(parent);
    copyParams(this, tool);
    return tool;
}

void MarkerTool::process(QPainter& painter, const QPixmap& pixmap)
{
    auto compositionMode = painter.compositionMode();
    qreal opacity = painter.opacity();
    auto pen = painter.pen();
    painter.setCompositionMode(QPainter::CompositionMode_Multiply);
    painter.setOpacity(0.35);
    painter.setPen(QPen(m_color, m_thickness));
    painter.drawLine(m_points.first, m_points.second);
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
    painter.setPen(QPen(context.color, PADDING_VALUE + context.thickness));
    painter.drawLine(context.mousePos, context.mousePos);
    painter.setPen(pen);
    painter.setOpacity(opacity);
    painter.setCompositionMode(compositionMode);
}

void MarkerTool::drawStart(const CaptureContext& context)
{
    m_color = context.color;
    m_thickness = context.thickness + PADDING_VALUE;
    m_points.first = context.mousePos;
    m_points.second = context.mousePos;
}

void MarkerTool::pressed(const CaptureContext& context)
{
    Q_UNUSED(context);
}

void MarkerTool::thicknessChanged(const int th)
{
    m_thickness = th + PADDING_VALUE;
}
