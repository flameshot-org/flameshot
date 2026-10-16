// SPDX-License-Identifier: GPL-3.0-or-later

#include "resizehandles.h"

#include <algorithm>
#include <utility>

namespace ResizeHandles {

namespace {

// Moves @p moving to @p to; when it lands on @p fixed, keeps it one pixel on
// the side it started from.
int placeEdge(int moving, int fixed, int to)
{
    if (to != fixed) {
        return to;
    }
    return moving < fixed ? fixed - 1 : fixed + 1;
}

struct Edges
{
    int left, top, right, bottom;
};

// @p box's edges after dragging @p handle to @p pos; not normalized, so a
// flipped box keeps its orientation.
Edges draggedEdges(const QRect& box, Handle handle, const QPoint& pos)
{
    Edges e{ box.left(), box.top(), box.right(), box.bottom() };
    if (handle == TopLeft || handle == Left || handle == BottomLeft) {
        e.left = placeEdge(e.left, e.right, pos.x());
    }
    if (handle == TopRight || handle == Right || handle == BottomRight) {
        e.right = placeEdge(e.right, e.left, pos.x());
    }
    if (handle == TopLeft || handle == Top || handle == TopRight) {
        e.top = placeEdge(e.top, e.bottom, pos.y());
    }
    if (handle == BottomLeft || handle == Bottom || handle == BottomRight) {
        e.bottom = placeEdge(e.bottom, e.top, pos.y());
    }
    return e;
}

// Maps @p v from [from1, from2] onto [to1, to2]; an empty source span has
// nothing to scale.
int rescale(int v, int from1, int from2, int to1, int to2)
{
    if (from1 == from2) {
        return v;
    }
    return to1 + qRound(double(v - from1) * (to2 - to1) / (from2 - from1));
}

int squaredDistance(const QPoint& a, const QPoint& b)
{
    const QPoint d = a - b;
    return QPoint::dotProduct(d, d);
}

}

QRect boxOf(const QPoint& first, const QPoint& second)
{
    return { QPoint(std::min(first.x(), second.x()),
                    std::min(first.y(), second.y())),
             QPoint(std::max(first.x(), second.x()),
                    std::max(first.y(), second.y())) };
}

QPoint handleCenter(const QRect& box, Handle handle)
{
    const QPoint c = box.center();
    switch (handle) {
        case TopLeft:
            return box.topLeft();
        case Top:
            return { c.x(), box.top() };
        case TopRight:
            return box.topRight();
        case Right:
            return { box.right(), c.y() };
        case BottomRight:
            return box.bottomRight();
        case Bottom:
            return { c.x(), box.bottom() };
        case BottomLeft:
            return box.bottomLeft();
        case Left:
            return { box.left(), c.y() };
        case None:
        case Start:
        case End:
            break;
    }
    return c;
}

Handle handleAt(const QRect& box, const QPoint& pos, int tolerance)
{
    if (box.isEmpty()) {
        return None;
    }
    // Corners first: on a small box they overlap the edge handles.
    for (Handle h : { TopLeft,
                      TopRight,
                      BottomRight,
                      BottomLeft,
                      Top,
                      Right,
                      Bottom,
                      Left }) {
        const QPoint d = pos - handleCenter(box, h);
        if (QPoint::dotProduct(d, d) <= tolerance * tolerance) {
            return h;
        }
    }
    return None;
}

QRect resized(const QRect& start, Handle handle, const QPoint& pos)
{
    auto [left, top, right, bottom] = draggedEdges(start, handle, pos);
    if (left > right) {
        std::swap(left, right);
    }
    if (top > bottom) {
        std::swap(top, bottom);
    }
    return { QPoint(left, top), QPoint(right, bottom) };
}

Handle endpointAt(const QPoint& first,
                  const QPoint& second,
                  const QPoint& pos,
                  int tolerance)
{
    const int reach = tolerance * tolerance;
    const int toFirst = squaredDistance(pos, first);
    const int toSecond = squaredDistance(pos, second);
    if (toSecond <= reach && toSecond <= toFirst) {
        return End;
    }
    if (toFirst <= reach) {
        return Start;
    }
    return None;
}

QPair<QPoint, QPoint> movedEndpoint(const QPoint& first,
                                    const QPoint& second,
                                    Handle handle,
                                    const QPoint& pos)
{
    auto nudged = [&pos](const QPoint& moving, const QPoint& fixed) {
        if (pos != fixed) {
            return pos;
        }
        return fixed + QPoint(moving.x() < fixed.x() ? -1 : 1, 0);
    };
    if (handle == Start) {
        return { nudged(first, second), second };
    }
    if (handle == End) {
        return { first, nudged(second, first) };
    }
    return { first, second };
}

QVector<QPoint> stretched(const QVector<QPoint>& points,
                          const QRect& box,
                          Handle handle,
                          const QPoint& pos)
{
    const Edges e = draggedEdges(box, handle, pos);
    QVector<QPoint> result;
    result.reserve(points.size());
    for (const QPoint& p : points) {
        result.append(
          { rescale(p.x(), box.left(), box.right(), e.left, e.right),
            rescale(p.y(), box.top(), box.bottom(), e.top, e.bottom) });
    }
    return result;
}

std::optional<Qt::CursorShape> objectCursor(Handle handle,
                                            bool movingObject,
                                            bool overObject)
{
    switch (handle) {
        case TopLeft:
        case BottomRight:
            return Qt::SizeFDiagCursor;
        case TopRight:
        case BottomLeft:
            return Qt::SizeBDiagCursor;
        case Left:
        case Right:
            return Qt::SizeHorCursor;
        case Top:
        case Bottom:
            return Qt::SizeVerCursor;
        case Start:
        case End:
            return Qt::SizeAllCursor;
        case None:
            break;
    }
    if (movingObject) {
        return Qt::ClosedHandCursor;
    }
    if (overObject) {
        return Qt::OpenHandCursor;
    }
    return {};
}

}
