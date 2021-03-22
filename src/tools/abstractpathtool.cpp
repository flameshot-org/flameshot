// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "abstractpathtool.h"

AbstractPathTool::AbstractPathTool(QObject* parent)
  : CaptureTool(parent)
  , m_thickness(0)
  , m_padding(0)
{}

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

void AbstractPathTool::drawEnd(const QPoint& p)
{
    Q_UNUSED(p);
}

void AbstractPathTool::drawMove(const QPoint& p)
{
    addPoint(p);
}

void AbstractPathTool::colorChanged(const QColor& c)
{
    m_color = c;
}

void AbstractPathTool::thicknessChanged(const int th)
{
    m_thickness = th;
}

void AbstractPathTool::updateBackup(const QPixmap& pixmap)
{
    m_pixmapBackup = pixmap.copy(backupRect(pixmap));
}

QRect AbstractPathTool::backupRect(const QPixmap& pixmap) const
{
    const QRect& limits = pixmap.rect();
#if defined(Q_OS_MACOS)
    // Not sure how will it work on 4k and fullHd on Linux or Windows with a
    // capture of different displays with different DPI, so let it be MacOS
    // specific only.
    const qreal pixelRatio = pixmap.devicePixelRatio();
    const int val = (m_thickness + m_padding) * pixelRatio;
    QRect r = m_backupArea.normalized();
    if (1 != pixelRatio) {
        r.moveTo(r.topLeft() * pixelRatio);
        r.setSize(r.size() * pixelRatio);
    }
#else
    const int val = m_thickness + m_padding;
    QRect r = m_backupArea.normalized();
#endif
    r += QMargins(val, val, val, val);
    return r.intersected(limits);
}

void AbstractPathTool::addPoint(const QPoint& point)
{
    if (m_backupArea.left() > point.x()) {
        m_backupArea.setLeft(point.x());
    } else if (m_backupArea.right() < point.x()) {
        m_backupArea.setRight(point.x());
    }
    if (m_backupArea.top() > point.y()) {
        m_backupArea.setTop(point.y());
    } else if (m_backupArea.bottom() < point.y()) {
        m_backupArea.setBottom(point.y());
    }
    m_points.append(point);
}
