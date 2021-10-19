// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "abstracttwopointtool.h"
#include <QCursor>
#include <QScreen>
#include <cmath>

namespace {

const double ADJ_UNIT = std::atan(1.0);
const int DIRS_NUMBER = 4;

enum UNIT
{
    HORIZ_DIR = 0,
    DIAG1_DIR = 1,
    VERT_DIR = 2
};

const double ADJ_DIAG_UNIT = 2 * ADJ_UNIT;
const int DIAG_DIRS_NUMBER = 2;

enum DIAG_UNIT
{
    DIR1 = 0
};

}

AbstractTwoPointTool::AbstractTwoPointTool(QObject* parent)
  : CaptureTool(parent)
  , m_thickness(1)
  , m_padding(0)
{}

void AbstractTwoPointTool::copyParams(const AbstractTwoPointTool* from,
                                      AbstractTwoPointTool* to)
{
    CaptureTool::copyParams(from, to);
    to->m_points.first = from->m_points.first;
    to->m_points.second = from->m_points.second;
    to->m_color = from->m_color;
    to->m_thickness = from->m_thickness;
    to->m_padding = from->m_padding;
    to->m_supportsOrthogonalAdj = from->m_supportsOrthogonalAdj;
    to->m_supportsDiagonalAdj = from->m_supportsDiagonalAdj;
}

bool AbstractTwoPointTool::isValid() const
{
    return (m_points.first != m_points.second);
}

bool AbstractTwoPointTool::closeOnButtonPressed() const
{
    return false;
}

bool AbstractTwoPointTool::isSelectable() const
{
    return true;
}

bool AbstractTwoPointTool::showMousePreview() const
{
    return true;
}

QRect AbstractTwoPointTool::mousePreviewRect(
  const CaptureContext& context) const
{
    QRect rect(0, 0, context.toolSize + 2, context.toolSize + 2);
    rect.moveCenter(context.mousePos);
    return rect;
}

QRect AbstractTwoPointTool::boundingRect() const
{
    if (!isValid()) {
        return {};
    }
    int offset =
      m_thickness <= 1 ? 1 : static_cast<int>(round(m_thickness * 0.7 + 0.5));
    QRect rect =
      QRect(std::min(m_points.first.x(), m_points.second.x()) - offset,
            std::min(m_points.first.y(), m_points.second.y()) - offset,
            std::abs(m_points.first.x() - m_points.second.x()) + offset * 2,
            std::abs(m_points.first.y() - m_points.second.y()) + offset * 2);

    return rect.normalized();
}

void AbstractTwoPointTool::drawEnd(const QPoint& p)
{
    Q_UNUSED(p)
}

void AbstractTwoPointTool::drawMove(const QPoint& p)
{
    m_points.second = p;
}

void AbstractTwoPointTool::drawMoveWithAdjustment(const QPoint& p)
{
    m_points.second = m_points.first + adjustedVector(p - m_points.first);
}

void AbstractTwoPointTool::onColorChanged(const QColor& c)
{
    m_color = c;
}

void AbstractTwoPointTool::onSizeChanged(int size)
{
    m_thickness = size;
}

void AbstractTwoPointTool::paintMousePreview(QPainter& painter,
                                             const CaptureContext& context)
{
    painter.setPen(QPen(context.color, context.toolSize));
    painter.drawLine(context.mousePos, context.mousePos);
}

void AbstractTwoPointTool::drawStart(const CaptureContext& context)
{
    onColorChanged(context.color);
    m_points.first = context.mousePos;
    m_points.second = context.mousePos;
    onSizeChanged(context.toolSize);
}

QPoint AbstractTwoPointTool::adjustedVector(QPoint v) const
{
    if (m_supportsOrthogonalAdj && m_supportsDiagonalAdj) {
        int dir = (static_cast<int>(round(atan2(-v.y(), v.x()) / ADJ_UNIT)) +
                   DIRS_NUMBER) %
                  DIRS_NUMBER;
        if (dir == UNIT::HORIZ_DIR) {
            v.setY(0);
        } else if (dir == UNIT::VERT_DIR) {
            v.setX(0);
        } else if (dir == UNIT::DIAG1_DIR) {
            int newX = (v.x() - v.y()) / 2;
            int newY = -newX;
            v.setX(newX);
            v.setY(newY);
        } else {
            int newX = (v.x() + v.y()) / 2;
            int newY = newX;
            v.setX(newX);
            v.setY(newY);
        }
    } else if (m_supportsDiagonalAdj) {
        int dir =
          (static_cast<int>(round((atan2(-v.y(), v.x()) - ADJ_DIAG_UNIT / 2) /
                                  ADJ_DIAG_UNIT)) +
           DIAG_DIRS_NUMBER) %
          DIAG_DIRS_NUMBER;
        if (dir == DIAG_UNIT::DIR1) {
            int newX = (v.x() - v.y()) / 2;
            int newY = -newX;
            v.setX(newX);
            v.setY(newY);
        } else {
            int newX = (v.x() + v.y()) / 2;
            int newY = newX;
            v.setX(newX);
            v.setY(newY);
        }
    }
    return v;
}

void AbstractTwoPointTool::move(const QPoint& pos)
{
    QPoint offset = m_points.second - m_points.first;
    m_points.first = pos;
    m_points.second = m_points.first + offset;
}

const QPoint* AbstractTwoPointTool::pos()
{
    return &m_points.first;
}
