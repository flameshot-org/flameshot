// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "zoomlenstool.h"

#include <QPainter>
#include <cmath>

namespace {
const int POPUP_GAP = 20;
const int POPUP_SCALE = 2;
const int RESIZE_MARGIN = 10;
const int HANDLE_RADIUS = 4;
const QColor DEFAULT_COLOR = QColor(255, 0, 0);
}

ZoomLensTool::ZoomLensTool(QObject* parent)
  : AbstractTwoPointTool(parent)
  , m_popupInitialized(false)
  , m_dragTarget(DragTarget::SOURCE_MOVE)
  , m_resizeCorner(Corner::BOTTOM_RIGHT)
  , m_aspectRatio(1.0)
{
    m_supportsDiagonalAdj = true;
}

QIcon ZoomLensTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor)
    return QIcon(iconPath(background) + "zoomlens.svg");
}

QString ZoomLensTool::name() const
{
    return tr("Zoom Lens");
}

CaptureTool::Type ZoomLensTool::type() const
{
    return CaptureTool::TYPE_ZOOMLENS;
}

QString ZoomLensTool::description() const
{
    return tr("Magnify a selected area");
}

QRect ZoomLensTool::sourceRect() const
{
    return QRect(points().first, points().second).normalized();
}

QRect ZoomLensTool::calcPopupRect(const QRect& source,
                                   const QSize& imageBounds) const
{
    int popupW = source.width() * POPUP_SCALE;
    int popupH = source.height() * POPUP_SCALE;

    // Try right
    QPoint pos(source.right() + POPUP_GAP, source.top());
    if (pos.x() + popupW <= imageBounds.width()) {
        return QRect(pos, QSize(popupW, popupH));
    }
    // Try left
    pos = QPoint(source.left() - POPUP_GAP - popupW, source.top());
    if (pos.x() >= 0) {
        return QRect(pos, QSize(popupW, popupH));
    }
    // Try below
    pos = QPoint(source.left(), source.bottom() + POPUP_GAP);
    if (pos.y() + popupH <= imageBounds.height()) {
        return QRect(pos, QSize(popupW, popupH));
    }
    // Try above
    pos = QPoint(source.left(), source.top() - POPUP_GAP - popupH);
    return QRect(pos, QSize(popupW, popupH));
}

QPoint ZoomLensTool::connectorStart(const QRect& source,
                                     const QRect& popup,
                                     int margin) const
{
    Q_UNUSED(margin)
    return source.center();
}

QPoint ZoomLensTool::connectorEnd(const QRect& source,
                                   const QRect& popup,
                                   int margin) const
{
    if (popup.left() > source.right()) {
        return QPoint(popup.left() - margin, popup.center().y());
    } else if (popup.right() < source.left()) {
        return QPoint(popup.right() + margin, popup.center().y());
    } else if (popup.top() > source.bottom()) {
        return QPoint(popup.center().x(), popup.top() - margin);
    } else {
        return QPoint(popup.center().x(), popup.bottom() + margin);
    }
}

ZoomLensTool::Corner ZoomLensTool::nearestCorner(const QPoint& pos,
                                                   const QRect& rect) const
{
    int dTL = (pos - rect.topLeft()).manhattanLength();
    int dTR = (pos - rect.topRight()).manhattanLength();
    int dBL = (pos - rect.bottomLeft()).manhattanLength();
    int dBR = (pos - rect.bottomRight()).manhattanLength();
    int minDist = dTL;
    Corner corner = Corner::TOP_LEFT;
    if (dTR < minDist) { minDist = dTR; corner = Corner::TOP_RIGHT; }
    if (dBL < minDist) { minDist = dBL; corner = Corner::BOTTOM_LEFT; }
    if (dBR < minDist) { corner = Corner::BOTTOM_RIGHT; }
    return corner;
}

bool ZoomLensTool::detectEdge(const QPoint& clickPos, const QRect& rect,
                               Edge& edge, QPoint& anchor,
                               int& fixedVal) const
{
    QPoint midTop((rect.left() + rect.right()) / 2, rect.top());
    QPoint midBottom((rect.left() + rect.right()) / 2, rect.bottom());
    QPoint midLeft(rect.left(), (rect.top() + rect.bottom()) / 2);
    QPoint midRight(rect.right(), (rect.top() + rect.bottom()) / 2);

    int dT = (clickPos - midTop).manhattanLength();
    int dB = (clickPos - midBottom).manhattanLength();
    int dL = (clickPos - midLeft).manhattanLength();
    int dR = (clickPos - midRight).manhattanLength();
    int minDist = qMin(qMin(dT, dB), qMin(dL, dR));

    if (minDist > RESIZE_MARGIN * 2) {
        return false;
    }

    if (minDist == dT) {
        edge = Edge::TOP;
        anchor = midTop;
        fixedVal = rect.bottom();
    } else if (minDist == dB) {
        edge = Edge::BOTTOM;
        anchor = midBottom;
        fixedVal = rect.top();
    } else if (minDist == dL) {
        edge = Edge::LEFT;
        anchor = midLeft;
        fixedVal = rect.right();
    } else {
        edge = Edge::RIGHT;
        anchor = midRight;
        fixedVal = rect.left();
    }
    return true;
}

void ZoomLensTool::drawHandles(QPainter& painter, const QRect& rect) const
{
    QColor drawColor = DEFAULT_COLOR;
    painter.setPen(QPen(Qt::black, 1));
    painter.setBrush(drawColor);
    // Corners
    painter.drawEllipse(rect.topLeft(), HANDLE_RADIUS, HANDLE_RADIUS);
    painter.drawEllipse(rect.topRight(), HANDLE_RADIUS, HANDLE_RADIUS);
    painter.drawEllipse(rect.bottomLeft(), HANDLE_RADIUS, HANDLE_RADIUS);
    painter.drawEllipse(rect.bottomRight(), HANDLE_RADIUS, HANDLE_RADIUS);
    // Midpoints
    QPoint midTop((rect.left() + rect.right()) / 2, rect.top());
    QPoint midBottom((rect.left() + rect.right()) / 2, rect.bottom());
    QPoint midLeft(rect.left(), (rect.top() + rect.bottom()) / 2);
    QPoint midRight(rect.right(), (rect.top() + rect.bottom()) / 2);
    painter.drawEllipse(midTop, HANDLE_RADIUS, HANDLE_RADIUS);
    painter.drawEllipse(midBottom, HANDLE_RADIUS, HANDLE_RADIUS);
    painter.drawEllipse(midLeft, HANDLE_RADIUS, HANDLE_RADIUS);
    painter.drawEllipse(midRight, HANDLE_RADIUS, HANDLE_RADIUS);
}

QRect ZoomLensTool::boundingRect() const
{
    if (!isValid()) {
        return {};
    }
    QRect source = sourceRect();
    if (!m_popupInitialized) {
        return source;
    }
    int offset = qMax(size(), 1) + HANDLE_RADIUS;
    return source.united(m_popupRect).adjusted(-offset, -offset, offset, offset);
}

CaptureTool* ZoomLensTool::copy(QObject* parent)
{
    auto* tool = new ZoomLensTool(parent);
    copyParams(this, tool);
    tool->m_popupRect = m_popupRect;
    tool->m_imageBounds = m_imageBounds;
    tool->m_popupInitialized = m_popupInitialized;
    tool->m_aspectRatio = m_aspectRatio;
    return tool;
}

void ZoomLensTool::process(QPainter& painter, const QPixmap& pixmap)
{
    QRect source = sourceRect();
    if (source.isEmpty()) {
        return;
    }

    QColor drawColor = color().isValid() ? color() : DEFAULT_COLOR;
    int thickness = qMax(size(), 1);
    QPen pen(drawColor, thickness, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);

    if (!m_popupInitialized) {
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(source);
        return;
    }

    // 1. Draw connector line, clipped so it hides behind source rect
    int margin = thickness / 2 + 1;
    QPoint start = connectorStart(source, m_popupRect, margin);
    QPoint end = connectorEnd(source, m_popupRect, margin);
    painter.save();
    QRect sourceOuter = source.adjusted(-margin, -margin, margin, margin);
    painter.setClipRegion(QRegion(0, 0, 99999, 99999) - QRegion(sourceOuter));
    painter.setPen(pen);
    painter.drawLine(start, end);
    painter.restore();

    // 2. Draw popup: zoomed content
    auto pixelRatio = pixmap.devicePixelRatio();
    QRect sourceScaled(source.topLeft() * pixelRatio,
                       source.size() * pixelRatio);
    QPixmap cropped = pixmap.copy(sourceScaled);
    QPixmap scaled = cropped.scaled(m_popupRect.size() * pixelRatio,
                                     Qt::IgnoreAspectRatio,
                                     Qt::SmoothTransformation);
    scaled.setDevicePixelRatio(pixelRatio);
    painter.drawPixmap(m_popupRect, scaled);

    // 3. Draw popup border
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(m_popupRect);

    // 4. Draw source area border
    painter.drawRect(source);
}

void ZoomLensTool::drawObjectSelection(QPainter& painter)
{
    if (!m_popupInitialized) {
        return;
    }
    // Draw corner handles instead of the default selection rectangle
    drawHandles(painter, sourceRect());
    drawHandles(painter, m_popupRect);
}

void ZoomLensTool::paintMousePreview(QPainter& painter,
                                      const CaptureContext& context)
{
    Q_UNUSED(context)
    Q_UNUSED(painter)
}

void ZoomLensTool::prepareMove(const QPoint& clickPos)
{
    if (!m_popupInitialized) {
        m_dragTarget = DragTarget::SOURCE_MOVE;
        m_moveAnchor = points().first;
        return;
    }

    QRect source = sourceRect();

    // Check popup corners first (resize popup)
    {
        int dTL = (clickPos - m_popupRect.topLeft()).manhattanLength();
        int dTR = (clickPos - m_popupRect.topRight()).manhattanLength();
        int dBL = (clickPos - m_popupRect.bottomLeft()).manhattanLength();
        int dBR = (clickPos - m_popupRect.bottomRight()).manhattanLength();
        int minDist = qMin(qMin(dTL, dTR), qMin(dBL, dBR));

        if (minDist <= RESIZE_MARGIN * 2) {
            m_dragTarget = DragTarget::POPUP_RESIZE;
            m_resizeCorner = nearestCorner(clickPos, m_popupRect);
            m_aspectRatio = static_cast<qreal>(m_popupRect.width()) /
                            qMax(m_popupRect.height(), 1);
            switch (m_resizeCorner) {
            case Corner::TOP_LEFT:
                m_moveAnchor = m_popupRect.topLeft();
                m_fixedCorner = m_popupRect.bottomRight();
                break;
            case Corner::TOP_RIGHT:
                m_moveAnchor = m_popupRect.topRight();
                m_fixedCorner = m_popupRect.bottomLeft();
                break;
            case Corner::BOTTOM_LEFT:
                m_moveAnchor = m_popupRect.bottomLeft();
                m_fixedCorner = m_popupRect.topRight();
                break;
            case Corner::BOTTOM_RIGHT:
                m_moveAnchor = m_popupRect.bottomRight();
                m_fixedCorner = m_popupRect.topLeft();
                break;
            }
            return;
        }
    }

    // Check source corners (resize source)
    {
        int dTL = (clickPos - source.topLeft()).manhattanLength();
        int dTR = (clickPos - source.topRight()).manhattanLength();
        int dBL = (clickPos - source.bottomLeft()).manhattanLength();
        int dBR = (clickPos - source.bottomRight()).manhattanLength();
        int minDist = qMin(qMin(dTL, dTR), qMin(dBL, dBR));

        if (minDist <= RESIZE_MARGIN * 2) {
            m_dragTarget = DragTarget::SOURCE_RESIZE;
            m_resizeCorner = nearestCorner(clickPos, source);
            switch (m_resizeCorner) {
            case Corner::TOP_LEFT:
                m_moveAnchor = source.topLeft();
                m_fixedCorner = source.bottomRight();
                break;
            case Corner::TOP_RIGHT:
                m_moveAnchor = source.topRight();
                m_fixedCorner = source.bottomLeft();
                break;
            case Corner::BOTTOM_LEFT:
                m_moveAnchor = source.bottomLeft();
                m_fixedCorner = source.topRight();
                break;
            case Corner::BOTTOM_RIGHT:
                m_moveAnchor = source.bottomRight();
                m_fixedCorner = source.topLeft();
                break;
            }
            return;
        }
    }

    // Check popup edge midpoints (edge resize popup)
    {
        Edge edge;
        QPoint anchor;
        int fixedVal;
        if (detectEdge(clickPos, m_popupRect, edge, anchor, fixedVal)) {
            m_dragTarget = DragTarget::POPUP_EDGE_RESIZE;
            m_resizeEdge = edge;
            m_moveAnchor = anchor;
            m_fixedEdgeVal = fixedVal;
            return;
        }
    }

    // Check source edge midpoints (edge resize source)
    {
        Edge edge;
        QPoint anchor;
        int fixedVal;
        if (detectEdge(clickPos, source, edge, anchor, fixedVal)) {
            m_dragTarget = DragTarget::SOURCE_EDGE_RESIZE;
            m_resizeEdge = edge;
            m_moveAnchor = anchor;
            m_fixedEdgeVal = fixedVal;
            return;
        }
    }

    // Inside popup → move popup only
    if (m_popupRect.contains(clickPos)) {
        m_dragTarget = DragTarget::POPUP_MOVE;
        m_moveAnchor = m_popupRect.topLeft();
        return;
    }

    // Default: move everything
    m_dragTarget = DragTarget::SOURCE_MOVE;
    m_moveAnchor = points().first;
}

const QPoint* ZoomLensTool::pos()
{
    if (!m_popupInitialized) {
        return AbstractTwoPointTool::pos();
    }

    switch (m_dragTarget) {
    case DragTarget::POPUP_MOVE:
        m_moveAnchor = m_popupRect.topLeft();
        return &m_moveAnchor;
    case DragTarget::POPUP_RESIZE:
    case DragTarget::SOURCE_RESIZE:
    case DragTarget::POPUP_EDGE_RESIZE:
    case DragTarget::SOURCE_EDGE_RESIZE:
        // m_moveAnchor was set in prepareMove
        return &m_moveAnchor;
    default:
        return AbstractTwoPointTool::pos();
    }
}

void ZoomLensTool::move(const QPoint& pos)
{
    switch (m_dragTarget) {
    case DragTarget::POPUP_MOVE:
        m_popupRect.moveTopLeft(pos);
        break;

    case DragTarget::POPUP_RESIZE: {
        QRect newRect = QRect(m_fixedCorner, pos).normalized();
        int newW = newRect.width();
        int newH = static_cast<int>(round(newW / m_aspectRatio));
        if (newH < 10) {
            newH = 10;
            newW = static_cast<int>(round(newH * m_aspectRatio));
        }
        if (newW < 10) newW = 10;

        switch (m_resizeCorner) {
        case Corner::TOP_LEFT:
            newRect = QRect(m_fixedCorner.x() - newW + 1,
                            m_fixedCorner.y() - newH + 1, newW, newH);
            break;
        case Corner::TOP_RIGHT:
            newRect = QRect(m_fixedCorner.x(),
                            m_fixedCorner.y() - newH + 1, newW, newH);
            break;
        case Corner::BOTTOM_LEFT:
            newRect = QRect(m_fixedCorner.x() - newW + 1,
                            m_fixedCorner.y(), newW, newH);
            break;
        case Corner::BOTTOM_RIGHT:
            newRect = QRect(m_fixedCorner.x(),
                            m_fixedCorner.y(), newW, newH);
            break;
        }
        m_popupRect = newRect;
        break;
    }

    case DragTarget::SOURCE_RESIZE: {
        // pos is the new position of the dragged source corner
        QRect newSource = QRect(m_fixedCorner, pos).normalized();
        if (newSource.width() < 5) newSource.setWidth(5);
        if (newSource.height() < 5) newSource.setHeight(5);
        // Update both points: move sets first, drawMove sets second
        AbstractTwoPointTool::move(newSource.topLeft());
        drawMove(newSource.bottomRight());
        // Update popup size to match new source proportions (keep center)
        QPoint popupCenter = m_popupRect.center();
        int newPopupW = newSource.width() * POPUP_SCALE;
        int newPopupH = newSource.height() * POPUP_SCALE;
        m_popupRect = QRect(0, 0, newPopupW, newPopupH);
        m_popupRect.moveCenter(popupCenter);
        m_aspectRatio = static_cast<qreal>(newPopupW) / qMax(newPopupH, 1);
        break;
    }

    case DragTarget::POPUP_EDGE_RESIZE: {
        QRect r = m_popupRect;
        switch (m_resizeEdge) {
        case Edge::TOP:    r.setTop(pos.y()); break;
        case Edge::BOTTOM: r.setBottom(pos.y()); break;
        case Edge::LEFT:   r.setLeft(pos.x()); break;
        case Edge::RIGHT:  r.setRight(pos.x()); break;
        }
        if (r.normalized().width() >= 10 && r.normalized().height() >= 10) {
            m_popupRect = r.normalized();
        }
        break;
    }

    case DragTarget::SOURCE_EDGE_RESIZE: {
        QRect s = sourceRect();
        switch (m_resizeEdge) {
        case Edge::TOP:    s.setTop(pos.y()); break;
        case Edge::BOTTOM: s.setBottom(pos.y()); break;
        case Edge::LEFT:   s.setLeft(pos.x()); break;
        case Edge::RIGHT:  s.setRight(pos.x()); break;
        }
        s = s.normalized();
        if (s.width() >= 5 && s.height() >= 5) {
            AbstractTwoPointTool::move(s.topLeft());
            drawMove(s.bottomRight());
            // Update popup to match new source proportions
            QPoint popupCenter = m_popupRect.center();
            int newPopupW = s.width() * POPUP_SCALE;
            int newPopupH = s.height() * POPUP_SCALE;
            m_popupRect = QRect(0, 0, newPopupW, newPopupH);
            m_popupRect.moveCenter(popupCenter);
            m_aspectRatio = static_cast<qreal>(newPopupW) / qMax(newPopupH, 1);
        }
        break;
    }

    default:
        // SOURCE_MOVE: move both source area and popup together
        QPoint oldPos = points().first;
        AbstractTwoPointTool::move(pos);
        if (m_popupInitialized) {
            QPoint delta = pos - oldPos;
            m_popupRect.translate(delta);
        }
        break;
    }
}

void ZoomLensTool::drawMove(const QPoint& p)
{
    AbstractTwoPointTool::drawMove(p);
    QRect source = sourceRect();
    if (source.width() > 5 && source.height() > 5) {
        m_popupRect = calcPopupRect(source, m_imageBounds);
        m_popupInitialized = true;
        m_aspectRatio = static_cast<qreal>(m_popupRect.width()) /
                        qMax(m_popupRect.height(), 1);
    }
}

void ZoomLensTool::drawStart(const CaptureContext& context)
{
    AbstractTwoPointTool::drawStart(context);
    if (!color().isValid()) {
        onColorChanged(DEFAULT_COLOR);
    }
    onSizeChanged(context.toolSize);
    m_popupInitialized = false;
    m_dragTarget = DragTarget::SOURCE_MOVE;
    m_imageBounds = context.screenshot.size() /
                    context.screenshot.devicePixelRatio();
}

void ZoomLensTool::drawEnd(const QPoint& p)
{
    AbstractTwoPointTool::drawEnd(p);
    QRect source = sourceRect();
    if (!source.isEmpty()) {
        m_popupRect = calcPopupRect(source, m_imageBounds);
        m_popupInitialized = true;
        m_aspectRatio = static_cast<qreal>(m_popupRect.width()) /
                        qMax(m_popupRect.height(), 1);
    }
}

void ZoomLensTool::pressed(CaptureContext& context)
{
    Q_UNUSED(context)
}
