// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "rectangletool.h"
#include <QPainter>

namespace {
#define PADDING_VALUE 2
}

RectangleTool::RectangleTool(QObject* parent)
  : AbstractTwoPointTool(parent)
{
    m_supportsDiagonalAdj = true;
    context_thickness = 0;
}

QIcon RectangleTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor)
    return QIcon(iconPath(background) + "square.svg");
}
QString RectangleTool::name() const
{
    return tr("Rectangle");
}

ToolType RectangleTool::nameID() const
{
    return ToolType::RECTANGLE;
}

QString RectangleTool::description() const
{
    return tr("Set the Rectangle as the paint tool");
}

CaptureTool* RectangleTool::copy(QObject* parent)
{
    auto* tool = new RectangleTool(parent);
    copyParams(this, tool);
    return tool;
}

void RectangleTool::process(QPainter& painter, const QPixmap& pixmap)
{
    Q_UNUSED(pixmap)
    QPen orig_pen = painter.pen();
    QBrush orig_brush = painter.brush();
    painter.setPen(
      QPen(color(), thickness(), Qt::SolidLine, Qt::SquareCap, Qt::RoundJoin));
    painter.setBrush(QBrush(color()));
    if (context_thickness == 0) {
        painter.drawRect(QRect(points().first, points().second));
    } else {
        painter.drawRoundedRect(
          std::min(points().first.x(), points().second.x()),
          std::min(points().first.y(), points().second.y()),
          std::abs(points().first.x() - points().second.x()),
          std::abs(points().first.y() - points().second.y()),
          thickness(),
          thickness());
    }
    painter.setPen(orig_pen);
    painter.setBrush(orig_brush);
}

void RectangleTool::paintMousePreview(QPainter& painter,
                                      const CaptureContext& context)
{
    painter.setPen(QPen(context.color, context.thickness));
    painter.drawLine(context.mousePos, context.mousePos);
}

void RectangleTool::drawStart(const CaptureContext& context)
{
    AbstractTwoPointTool::drawStart(context);
    context_thickness = context.thickness;
}

void RectangleTool::pressed(const CaptureContext& context)
{
    Q_UNUSED(context)
}
