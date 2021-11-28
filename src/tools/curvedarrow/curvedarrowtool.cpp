#include "curvedarrowtool.h"
#include <cmath>

namespace {
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

std::string getAngleDirection(QPointF start, QPointF end)
{
    std::string dir;

    if (start.y() > end.y())
        dir += "up";
    if (start.y() < end.y())
        dir += "down";
    if (start.x() < end.x())
        dir += "right";
    if (start.x() > end.x())
        dir += "left";
    return dir;
}

int getAngle(const std::string &dir)
{
    int ang = 0;

    if (dir == "upright")
        ang = 90 * 16;
    if (dir == "downright")
        ang = 180 * 16;
    if (dir == "downleft")
        ang = 270 * 16;
    if (dir == "upleft")
        ang = 360 * 16;
    return ang;
}

} // unnamed namespace

CurvedArrowTool::CurvedArrowTool(QObject* parent)
  : AbstractTwoPointTool(parent)
{
    setPadding(ArrowWidth / 2);
    m_supportsOrthogonalAdj = true;
    m_supportsDiagonalAdj = true;
}

QIcon CurvedArrowTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor)
    return QIcon(iconPath(background) + "arrow-bottom-left.svg");
}

QString CurvedArrowTool::name() const
{
    return tr("CurvedArrow");
}

CaptureTool::Type CurvedArrowTool::type() const
{
    return CaptureTool::TYPE_ARROW;//TYPE_CURVEDARROW;
}

QString CurvedArrowTool::description() const
{
    return tr("Set the CurvedArrow as the paint tool");
}

QRect CurvedArrowTool::boundingRect() const
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

CaptureTool* CurvedArrowTool::copy(QObject* parent)
{
    CurvedArrowTool* tool = new CurvedArrowTool(parent);
    copyParams(this, tool);
    return tool;
}

void CurvedArrowTool::copyParams(const CurvedArrowTool* from, CurvedArrowTool* to)
{
    AbstractTwoPointTool::copyParams(from, to);
    to->m_arrowPath = this->m_arrowPath;
}


void CurvedArrowTool::process(QPainter& painter, const QPixmap& pixmap)
{
    Q_UNUSED(pixmap)
    int startAngle = getAngle(getAngleDirection(points().first, points().second));//getAngleDirection(points().first, points().second); // 30 vers le bas // -150 vers le haut // 120 vers la droite // -60 vers la gauche
    int spanAngle = 120 * 16;
    QRectF rect(points().first, points().second);

    painter.setPen(QPen(color(), size()));
    painter.drawArc(rect, startAngle, spanAngle);
}


void CurvedArrowTool::pressed(CaptureContext& context)
{
    Q_UNUSED(context)
}
