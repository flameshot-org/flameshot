// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "rectangletool.h"
#include <QPainter>
#include <QPainterPath>
#include <cmath>

RectangleTool::RectangleTool(QObject* parent)
  : AbstractTwoPointTool(parent)
{
    m_supportsDiagonalAdj = true;
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
    if (thickness() == 0) {
        painter.drawRect(QRect(points().first, points().second));
    } else {
        QPainterPath path;
        int offset =
          thickness() <= 1 ? 1 : static_cast<int>(round(thickness() / 2 + 0.5));
        path.addRoundedRect(
          QRectF(
            std::min(points().first.x(), points().second.x()) - offset,
            std::min(points().first.y(), points().second.y()) - offset,
            std::abs(points().first.x() - points().second.x()) + offset * 2,
            std::abs(points().first.y() - points().second.y()) + offset * 2),
          thickness(),
          thickness());
        painter.fillPath(path, color());
    }
    painter.setPen(orig_pen);
    painter.setBrush(orig_brush);
}

void RectangleTool::drawStart(const CaptureContext& context)
{
    AbstractTwoPointTool::drawStart(context);
    thicknessChanged(context.thickness);
}

void RectangleTool::pressed(const CaptureContext& context)
{
    Q_UNUSED(context)
}
