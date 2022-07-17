// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "boldarrowtool.h"
#include <cmath>

namespace {
const int ArrowWidth = 10;
const int ArrowHeight = 18;

QPolygon drawArrow(QPoint p, const int length, const int weight)
{
    QPolygon poly;
    const int size = 8 * (1 + (weight - 1) / 2.0);

    poly << p << QPoint(p.x() + length - 2 * size, p.y() + 1.2 * size)
         << QPoint(p.x() + length - 2.3 * size, p.y() + 2 * size)
         << QPoint(p.x() + length, p.y())
         << QPoint(p.x() + length - 2.3 * size, p.y() - 2 * size)
         << QPoint(p.x() + length - 2 * size, p.y() - 1.2 * size) << p;

    return poly;
}

int distance(QPair<QPoint, QPoint> points)
{
    return std::sqrt(std::pow(points.first.x() - points.second.x(), 2) +
                     std::pow(points.first.y() - points.second.y(), 2));
}

float angleBetweenPoints(QPair<QPoint, QPoint> points)
{
    QPoint point = points.first - points.second;
    return std::atan2(point.y(), point.x()) * 180 / 3.14 + 180;
}

} // unnamed namespace

BoldArrowTool::BoldArrowTool(QObject* parent)
  : AbstractTwoPointTool(parent)
{
    setPadding(ArrowWidth / 2);
    m_supportsOrthogonalAdj = true;
    m_supportsDiagonalAdj = true;
}

QIcon BoldArrowTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor)
    return QIcon(iconPath(background) + "bold-arrow-bottom-left.svg");
}
QString BoldArrowTool::name() const
{
    return tr("Bold Arrow");
}

CaptureTool::Type BoldArrowTool::type() const
{
    return CaptureTool::TYPE_BOLD_ARROW;
}

QString BoldArrowTool::description() const
{
    return tr("Set the Bold Arrow as the paint tool");
}

QRect BoldArrowTool::boundingRect() const
{
    if (!isValid()) {
        return {};
    }

    int offset = size() <= 1 ? 1 : static_cast<int>(round(size() / 2 + 0.5));

    // get min and max arrow pos
    int min_x = points().first.x();
    int min_y = points().first.y();
    int max_x = points().first.x();
    int max_y = points().first.y();
    for (int i = 0; i < m_arrowPath.elementCount(); i++) {
        QPointF pt = m_arrowPath.elementAt(i);
        if (static_cast<int>(pt.x()) < min_x) {
            min_x = static_cast<int>(pt.x());
        }
        if (static_cast<int>(pt.y()) < min_y) {
            min_y = static_cast<int>(pt.y());
        }
        if (static_cast<int>(pt.x()) > max_x) {
            max_x = static_cast<int>(pt.x());
        }
        if (static_cast<int>(pt.y()) > max_y) {
            max_y = static_cast<int>(pt.y());
        }
    }

    // get min and max line pos
    int line_pos_min_x =
      std::min(std::min(points().first.x(), points().second.x()), min_x);
    int line_pos_min_y =
      std::min(std::min(points().first.y(), points().second.y()), min_y);
    int line_pos_max_x =
      std::max(std::max(points().first.x(), points().second.x()), max_x);
    int line_pos_max_y =
      std::max(std::max(points().first.y(), points().second.y()), max_y);

    QRect rect = QRect(line_pos_min_x - offset,
                       line_pos_min_y - offset,
                       line_pos_max_x - line_pos_min_x + offset * 2,
                       line_pos_max_y - line_pos_min_y + offset * 2);

    return rect.normalized();
}

CaptureTool* BoldArrowTool::copy(QObject* parent)
{
    auto* tool = new BoldArrowTool(parent);
    copyParams(this, tool);
    return tool;
}

void BoldArrowTool::copyParams(const BoldArrowTool* from, BoldArrowTool* to)
{
    AbstractTwoPointTool::copyParams(from, to);
    to->m_arrowPath = this->m_arrowPath;
}

void BoldArrowTool::process(QPainter& painter, const QPixmap& pixmap)
{
    Q_UNUSED(pixmap)
    QColor transparent = QColor(255, 255, 255, 0);
    painter.setPen(QPen(transparent, 1));

    float angle = angleBetweenPoints(points());

    QLinearGradient gradient(points().first, points().second);
    gradient.setColorAt(0, QColor(255, 255, 255, 0));
    gradient.setColorAt(1, color());
    painter.setBrush(gradient);

    QPolygon arrow = drawArrow(points().first, distance(points()), size());

    arrow = QTransform()
              .translate(points().first.x(), points().first.y())
              .rotate(angle)
              .translate(-points().first.x(), -points().first.y())
              .map(arrow);

    painter.drawPolygon(arrow);
}

void BoldArrowTool::pressed(CaptureContext& context)
{
    Q_UNUSED(context)
}
