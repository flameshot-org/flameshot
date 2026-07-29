// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "arrowtool.h"
#include "utils/confighandler.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>
#include <cmath>

namespace {
const int ArrowWidth = 10;
const int ArrowHeight = 18;
const int MinArrowStyle = 0;
const int MaxArrowStyle = 1;

bool isValidArrowStyle(int style)
{
    return style >= MinArrowStyle && style <= MaxArrowStyle;
}

QPainterPath getArrowHead(QPoint p1, QPoint p2, const int thickness)
{
    QLineF base(p1, p2);
    // Create the vector for the position of the base  of the arrowhead
    QLineF temp(QPoint(0, 0), p2 - p1);
    int val = ArrowHeight + thickness * 4;
    if (base.length() < (val - thickness * 2)) {
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
    if (l.length() < (val - thickness * 2)) {
        // here should be 0, but then we lose "angle", so this is hack, but
        // looks not very bad
        val = thickness / 4;
        l.setLength(val);
    } else {
        l.setLength(l.length() + thickness * 2 - val);
    }
    return l.toLine();
}

QPainterPath getCurvedArrowHead(QPointF p1, QPointF p2, const int thickness)
{
    QLineF line(p1, p2);
    if (line.length() <= 0) {
        return {};
    }

    const QPointF direction = (p2 - p1) / line.length();
    const QPointF normal(-direction.y(), direction.x());
    const QPointF baseCenter =
      getShorterLine(p1.toPoint(), p2.toPoint(), thickness).p2();
    const qreal halfWidth = (ArrowWidth + thickness * 2) / 2.0;
    const qreal baseDistance = QLineF(baseCenter, p2).length();
    const qreal notchDepth = std::min<qreal>(baseDistance * 0.45, halfWidth);

    const QPointF baseLeft = baseCenter + normal * halfWidth;
    const QPointF baseRight = baseCenter - normal * halfWidth;
    const QPointF notch = baseCenter + direction * notchDepth;
    const QPointF leftControl = baseCenter + normal * halfWidth * 0.25;
    const QPointF rightControl = baseCenter - normal * halfWidth * 0.25;

    QPainterPath path;
    path.moveTo(p2);
    path.lineTo(baseLeft);
    path.quadTo(leftControl, notch);
    path.quadTo(rightControl, baseRight);
    path.lineTo(p2);
    return path;
}

QLineF getCurvedArrowShaft(QPointF p1, QPointF p2, const int thickness)
{
    QLineF line(p1, p2);
    if (line.length() <= 0) {
        return {};
    }

    const QPointF direction = (p2 - p1) / line.length();
    QLineF shaft(getShorterLine(p1.toPoint(), p2.toPoint(), thickness));
    const qreal notchDepth =
      std::min<qreal>(QLineF(shaft.p2(), p2).length() * 0.45,
                      (ArrowWidth + thickness * 2) / 2.0);
    constexpr qreal overlap = 1.0;

    // The curved head has a concave back, so extend the straight shaft
    // slightly into the head to avoid a visible gap without leaking past
    // the head outline at large thicknesses.
    shaft.setP2(shaft.p2() + direction * (notchDepth + overlap));
    return shaft;
}

} // unnamed namespace

ArrowTool::ArrowTool(QObject* parent)
  : AbstractTwoPointTool(parent)
{
    const int configuredArrowStyle = ConfigHandler().arrowStyle();
    if (isValidArrowStyle(configuredArrowStyle)) {
        m_arrowStyle = static_cast<ArrowStyle>(configuredArrowStyle);
    }

    setPadding(ArrowWidth / 2);
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

CaptureTool::Type ArrowTool::type() const
{
    return CaptureTool::TYPE_ARROW;
}

QString ArrowTool::description() const
{
    return tr("Set the Arrow as the paint tool");
}

QRect ArrowTool::boundingRect() const
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

QWidget* ArrowTool::configurationWidget()
{
    auto* widget = new QWidget();
    auto* layout = new QHBoxLayout(widget);
    auto* label = new QLabel(tr("Arrow style:"), widget);
    auto* styleSelector = new QComboBox(widget);

    styleSelector->addItem(tr("Default"));
    styleSelector->addItem(tr("Curved"));
    styleSelector->setCurrentIndex(static_cast<int>(m_arrowStyle));
    connect(styleSelector,
            qOverload<int>(&QComboBox::currentIndexChanged),
            this,
            &ArrowTool::setArrowStyle);

    layout->addWidget(label);
    layout->addWidget(styleSelector);

    return widget;
}

CaptureTool* ArrowTool::copy(QObject* parent)
{
    auto* tool = new ArrowTool(parent);
    copyParams(this, tool);
    return tool;
}

void ArrowTool::copyParams(const ArrowTool* from, ArrowTool* to)
{
    AbstractTwoPointTool::copyParams(from, to);
    to->m_arrowPath = from->m_arrowPath;
    to->m_arrowStyle = from->m_arrowStyle;
}

void ArrowTool::process(QPainter& painter, const QPixmap& pixmap)
{
    bool isArrowReversed = ConfigHandler().reverseArrow();

    const QPoint& head = isArrowReversed ? points().second : points().first;
    const QPoint& tail = isArrowReversed ? points().first : points().second;

    Q_UNUSED(pixmap)
    painter.setPen(QPen(color(), size()));
    if (m_arrowStyle == ArrowStyle::Default) {
        painter.drawLine(getShorterLine(head, tail, size()));
        m_arrowPath = getArrowHead(head, tail, size());
        painter.fillPath(m_arrowPath, QBrush(color()));
        return;
    }

    painter.setPen(QPen(color(), size(), Qt::SolidLine, Qt::FlatCap));
    painter.drawLine(getCurvedArrowShaft(head, tail, size()));
    m_arrowPath = getCurvedArrowHead(head, tail, size());
    painter.fillPath(m_arrowPath, QBrush(color()));
}

void ArrowTool::pressed(CaptureContext& context)
{
    Q_UNUSED(context)
}

void ArrowTool::setArrowStyle(int style)
{
    if (!isValidArrowStyle(style)) {
        style = static_cast<int>(ArrowStyle::Default);
    }
    m_arrowStyle = static_cast<ArrowStyle>(style);
    ConfigHandler().setArrowStyle(style);
}
