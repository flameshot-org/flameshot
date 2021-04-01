// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "arrowtool.h"

namespace {

#define PADDING_VALUE 2

const int ArrowWidth = 10;
const int ArrowHeight = 18;

QPainterPath getArrowHead(QPoint p1, QPoint p2, const int thickness)
{
    QLineF base(p1, p2);
    // Create the vector for the position of the base  of the arrowhead
    QLineF temp(QPoint(0, 0), p2 - p1);
    int val = ArrowHeight + thickness * 4;
    if (base.length() < val) {
        val = static_cast<int>(base.length() + thickness * 2);
    }
    temp.setLength(base.length() + thickness * 2 - val);
    // Move across the line up to the head
    QPointF bottomTranslation(temp.p2());

    // Rotate base of the arrowhead
    base.setLength(ArrowWidth + thickness * 2);
    base.setAngle(base.angle() + 90);
    // Move to the correct point
    QPointF temp2 = p1 - base.p2();
    // Center it
    QPointF centerTranslation((temp2.x() / 2), (temp2.y() / 2));

    base.translate(bottomTranslation);
    base.translate(centerTranslation);

    QPainterPath path;
    path.moveTo(p2);
    path.lineTo(base.p1());
    path.lineTo(base.p2());
    path.lineTo(p2);
    return path;
}

// gets a shorter line to prevent overlap in the point of the arrow
QLine getShorterLine(QPoint p1, QPoint p2, const int thickness)
{
    QLineF l(p1, p2);
    int val = ArrowHeight + thickness * 4;
    if (l.length() < val) {
        val = static_cast<int>(l.length() + thickness * 2);
    }
    l.setLength(l.length() + thickness * 2 - val);
    return l.toLine();
}

} // unnamed namespace

ArrowTool::ArrowTool(QObject* parent)
  : AbstractTwoPointTool(parent)
{
    m_padding = ArrowWidth / 2;
    m_supportsOrthogonalAdj = true;
    m_supportsDiagonalAdj = true;
}

QIcon ArrowTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor)
    return QIcon(iconPath(background) + "arrow-bottom-left.svg");
}
QString ArrowTool::name() const
{
    return tr("Arrow");
}

ToolType ArrowTool::nameID() const
{
    return ToolType::ARROW;
}

QString ArrowTool::description() const
{
    return tr("Set the Arrow as the paint tool");
}

CaptureTool* ArrowTool::copy(QObject* parent)
{
    return new ArrowTool(parent);
}

void ArrowTool::process(QPainter& painter, const QPixmap& pixmap)
{
    Q_UNUSED(pixmap)
    painter.setPen(QPen(m_color, m_thickness));
    painter.drawLine(
      getShorterLine(m_points.first, m_points.second, m_thickness));
    painter.fillPath(getArrowHead(m_points.first, m_points.second, m_thickness),
                     QBrush(m_color));
}

void ArrowTool::paintMousePreview(QPainter& painter,
                                  const CaptureContext& context)
{
    painter.setPen(QPen(context.color, PADDING_VALUE + context.thickness));
    painter.drawLine(context.mousePos, context.mousePos);
}

void ArrowTool::drawStart(const CaptureContext& context)
{
    m_color = context.color;
    m_thickness = context.thickness + PADDING_VALUE;
    m_points.first = context.mousePos;
    m_points.second = context.mousePos;
}

void ArrowTool::pressed(const CaptureContext& context)
{
    Q_UNUSED(context)
}

void ArrowTool::drawObjectSelection(QPainter& painter)
{
    int offset =
      m_thickness <= 1 ? 1 : static_cast<int>(round(m_thickness / 2 + 0.5));
    QRect rect =
      QRect(std::min(m_points.first.x(), m_points.second.x()) - offset,
            std::min(m_points.first.y(), m_points.second.y()) - offset,
            std::abs(m_points.first.x() - m_points.second.x()) + offset * 2,
            std::abs(m_points.first.y() - m_points.second.y()) + offset * 2);

    if (rect.width() < ArrowWidth + m_thickness) {
        rect.setX(rect.x() - (ArrowWidth / 2 + offset));
        rect.setWidth(rect.width() + (ArrowWidth / 2 + offset));
    }
    if (rect.height() < ArrowWidth + m_thickness) {
        rect.setY(rect.y() - (ArrowWidth / 2 + offset));
        rect.setHeight(rect.height() + (ArrowWidth / 2 + offset));
    }
    drawObjectSelectionRect(painter, rect);
}
