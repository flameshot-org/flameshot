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
    VERT_DIR = 2,
    DIAG2_DIR = 3
};

const double ADJ_DIAG_UNIT = 2 * ADJ_UNIT;
const int DIAG_DIRS_NUMBER = 2;

enum DIAG_UNIT
{
    DIR1 = 0,
    DIR2 = 1
};

}

AbstractTwoPointTool::AbstractTwoPointTool(QObject* parent)
  : CaptureTool(parent)
  , m_thickness(0)
  , m_padding(0)
{}

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

void AbstractTwoPointTool::undo(QPixmap& pixmap)
{
    QPainter p(&pixmap);
#if defined(Q_OS_MACOS)
    // Not sure how will it work on 4k and fullHd on Linux or Windows with a
    // capture of different displays with different DPI, so let it be MacOS
    // specific only.
    const qreal pixelRatio = pixmap.devicePixelRatio();
    p.drawPixmap(backupRect(pixmap).topLeft() / pixelRatio, m_pixmapBackup);
#else
    p.drawPixmap(backupRect(pixmap).topLeft(), m_pixmapBackup);
#endif
    if (this->nameID() == ToolType::CIRCLECOUNT) {
        emit requestAction(REQ_DECREMENT_CIRCLE_COUNT);
    }
}

void AbstractTwoPointTool::drawEnd(const QPoint& p)
{
    Q_UNUSED(p);
}

void AbstractTwoPointTool::drawMove(const QPoint& p)
{
    m_points.second = p;
}

void AbstractTwoPointTool::drawMoveWithAdjustment(const QPoint& p)
{
    m_points.second = m_points.first + adjustedVector(p - m_points.first);
}

void AbstractTwoPointTool::colorChanged(const QColor& c)
{
    m_color = c;
}

void AbstractTwoPointTool::thicknessChanged(const int th)
{
    m_thickness = th;
}

void AbstractTwoPointTool::updateBackup(const QPixmap& pixmap)
{
    m_pixmapBackup = pixmap.copy(backupRect(pixmap));
}

QRect AbstractTwoPointTool::backupRect(const QPixmap& pixmap) const
{
    const QRect& limits = pixmap.rect();
    QRect r = QRect(m_points.first, m_points.second).normalized();
#if defined(Q_OS_MACOS)
    // Not sure how will it work on 4k and fullHd on Linux or Windows with a
    // capture of different displays with different DPI, so let it be MacOS
    // specific only.
    const qreal pixelRatio = pixmap.devicePixelRatio();
    if (1 != pixelRatio) {
        r.moveTo(r.topLeft() * pixelRatio);
        r.setSize(r.size() * pixelRatio);
    }
    const int val = (m_thickness + m_padding) * pixelRatio;
#else
    const int val = (m_thickness + m_padding);
#endif
    r += QMargins(val, val, val, val);
    return r.intersected(limits);
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
