// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QPair>
#include <QPoint>
#include <QRect>
#include <QVector>
#include <Qt>

#include <optional>

/// Geometry of the eight handles shown on a selected box-shaped object: four
/// corners and four edge midpoints. QtCore only, so it is unit-testable
/// headless.
namespace ResizeHandles {

enum Handle
{
    None,
    TopLeft,
    Top,
    TopRight,
    Right,
    BottomRight,
    Bottom,
    BottomLeft,
    Left,
    // Ends of a line-shaped object
    Start,
    End
};

/// The box with @p first and @p second as opposite corners, in either order.
QRect boxOf(const QPoint& first, const QPoint& second);

QPoint handleCenter(const QRect& box, Handle handle);

/// The handle within @p tolerance of @p pos, or None. Corners win over edges
/// when both are in reach, as on a box too small to tell them apart.
Handle handleAt(const QRect& box, const QPoint& pos, int tolerance);

/// @p start with the edges that @p handle controls moved to @p pos, the
/// opposite ones staying put. Dragging past the opposite side flips the box.
/// Opposite edges never coincide, so the result always spans two points.
QRect resized(const QRect& start, Handle handle, const QPoint& pos);

/// Start or End if within @p tolerance of that end, the nearer one when both
/// are; None otherwise.
Handle endpointAt(const QPoint& first,
                  const QPoint& second,
                  const QPoint& pos,
                  int tolerance);

/// The line with the end @p handle names moved to @p pos. An end dropped on
/// the other one is kept a pixel away, so the line stays valid.
QPair<QPoint, QPoint> movedEndpoint(const QPoint& first,
                                    const QPoint& second,
                                    Handle handle,
                                    const QPoint& pos);

/// @p points, whose bounds are @p box, scaled to follow @p box's @p handle
/// dragged to @p pos. Dragging past the opposite side mirrors them; an axis
/// with no extent to scale is left alone.
QVector<QPoint> stretched(const QVector<QPoint>& points,
                          const QRect& box,
                          Handle handle,
                          const QPoint& pos);

/// Cursor for a selected object: a resize cursor over @p handle (hovered or
/// being dragged), a hand while @p movingObject or over the object. Empty
/// when none of these apply.
std::optional<Qt::CursorShape> objectCursor(Handle handle,
                                            bool movingObject,
                                            bool overObject);

}
