// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "abstractpathtool.h"
#include <cmath>

AbstractPathTool::AbstractPathTool(QObject* parent)
  : CaptureTool(parent)
  , m_thickness(1)
  , m_padding(0)
{}

void AbstractPathTool::copyParams(const AbstractPathTool* from,
                                  AbstractPathTool* to)
{
    to->m_color = from->m_color;
    to->m_thickness = from->m_thickness;
    to->m_padding = from->m_padding;
    to->m_pos = from->m_pos;

    to->m_points.clear();
    for (auto point : from->m_points) {
        to->m_points.append(point);
    }
}

bool AbstractPathTool::isValid() const
{
    return m_points.length() > 1;
}

bool AbstractPathTool::closeOnButtonPressed() const
{
    return false;
}

bool AbstractPathTool::isSelectable() const
{
    return true;
}

bool AbstractPathTool::showMousePreview() const
{
    return true;
}

QRect AbstractPathTool::mousePreviewRect(const CaptureContext& context) const
{
    QRect rect(0, 0, context.toolSize + 2, context.toolSize + 2);
    rect.moveCenter(context.mousePos);
    return rect;
}

QRect AbstractPathTool::boundingRect() const
{
    if (m_points.isEmpty()) {
        return {};
    }
    int min_x = m_points.at(0).x();
    int min_y = m_points.at(0).y();
    int max_x = m_points.at(0).x();
    int max_y = m_points.at(0).y();
    for (auto point : m_points) {
        if (point.x() < min_x) {
            min_x = point.x();
        }
        if (point.y() < min_y) {
            min_y = point.y();
        }
        if (point.x() > max_x) {
            max_x = point.x();
        }
        if (point.y() > max_y) {
            max_y = point.y();
        }
    }

    int offset =
      m_thickness <= 1 ? 1 : static_cast<int>(round(m_thickness * 0.7 + 0.5));
    return QRect(min_x - offset,
                 min_y - offset,
                 std::abs(min_x - max_x) + offset * 2,
                 std::abs(min_y - max_y) + offset * 2)
      .normalized();
}

void AbstractPathTool::drawEnd(const QPoint& p)
{
    Q_UNUSED(p)
}

void AbstractPathTool::drawMove(const QPoint& p)
{
    addPoint(p);
}

void AbstractPathTool::onColorChanged(const QColor& c)
{
    m_color = c;
}

void AbstractPathTool::onSizeChanged(int size)
{
    m_thickness = size;
}

void AbstractPathTool::addPoint(const QPoint& point)
{
    if (m_pathArea.left() > point.x()) {
        m_pathArea.setLeft(point.x());
    } else if (m_pathArea.right() < point.x()) {
        m_pathArea.setRight(point.x());
    }
    if (m_pathArea.top() > point.y()) {
        m_pathArea.setTop(point.y());
    } else if (m_pathArea.bottom() < point.y()) {
        m_pathArea.setBottom(point.y());
    }
    m_points.append(point);
}

void AbstractPathTool::move(const QPoint& mousePos)
{
    if (m_points.empty()) {
        return;
    }
    QPoint basePos = *pos();
    QPoint offset = mousePos - basePos;
    for (auto& m_point : m_points) {
        m_point += offset;
    }
}

const QPoint* AbstractPathTool::pos()
{
    if (m_points.empty()) {
        m_pos = QPoint();
        return &m_pos;
    }
    int x = m_points.at(0).x();
    int y = m_points.at(0).y();
    for (auto point : m_points) {
        if (point.x() < x) {
            x = point.x();
        }
        if (point.y() < y) {
            y = point.y();
        }
    }
    m_pos.setX(x);
    m_pos.setY(y);
    return &m_pos;
}
