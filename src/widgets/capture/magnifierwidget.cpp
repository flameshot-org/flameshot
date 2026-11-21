// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "magnifierwidget.h"

#include <QApplication>
#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPixmap>
#include <QtMath>

MagnifierWidget::MagnifierWidget(const QPixmap& p,
                                 const QColor& c,
                                 bool isSquare,
                                 QWidget* parent)
  : QWidget(parent)
  , m_square(isSquare)
  , m_color(c)
  , m_borderColor(c)
  , m_screenshot(p)
{
    setFixedSize(parent->width(), parent->height());
    setAttribute(Qt::WA_TransparentForMouseEvents);
    m_color.setAlpha(130);
}

QPointF MagnifierWidget::magnifierCenter(const QPoint& localPos) const
{
    const qreal halfSpan = m_pixels * magZoom / 2.0;
    qreal drawPosX = localPos.x() + m_magOffset + halfSpan;
    if (drawPosX > width() - halfSpan) {
        drawPosX = localPos.x() - m_magOffset - halfSpan;
    }
    qreal drawPosY = localPos.y() + m_magOffset + halfSpan;
    if (drawPosY > height() - halfSpan) {
        drawPosY = localPos.y() - m_magOffset - halfSpan;
    }
    return { drawPosX, drawPosY };
}

void MagnifierWidget::updateCursorPos(const QPoint& localPos)
{
    m_cursorPos = localPos;
    m_hasCursorPos = true;

    const QPointF center = magnifierCenter(localPos);
    const qreal halfSpan = m_pixels * magZoom / 2.0;
    constexpr int pad = 8;
    const QRect newDrawRect(qFloor(center.x() - halfSpan) - pad,
                            qFloor(center.y() - halfSpan) - pad,
                            qCeil(halfSpan * 2.0) + pad * 2,
                            qCeil(halfSpan * 2.0) + pad * 2);

    if (!m_lastDrawRect.isEmpty()) {
        update(m_lastDrawRect);
    }
    update(newDrawRect);
    m_lastDrawRect = newDrawRect;
}

void MagnifierWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    if (!p.isActive()) {
        return;
    }
    if (m_square) {
        drawMagnifier(p);
    } else {
        drawMagnifierCircle(p);
    }
}

void MagnifierWidget::drawMagnifierCircle(QPainter& painter)
{
    const QPoint translated =
      m_hasCursorPos ? m_cursorPos : QWidget::mapFromGlobal(QCursor::pos());
    const qreal dpr = m_screenshot.devicePixelRatio();
    const int srcCenterX = qRound(translated.x() * dpr);
    const int srcCenterY = qRound(translated.y() * dpr);

    QPixmap patch(m_pixels, m_pixels);
    patch.fill(Qt::black);
    const QRect desired(
      srcCenterX - m_magPixels, srcCenterY - m_magPixels, m_pixels, m_pixels);
    const QRect avail = desired.intersected(
      QRect(0, 0, m_screenshot.width(), m_screenshot.height()));
    if (!avail.isEmpty()) {
        QPainter patchPainter(&patch);
        patchPainter.drawPixmap(QRectF(avail.left() - desired.left(),
                                       avail.top() - desired.top(),
                                       avail.width(),
                                       avail.height()),
                                m_screenshot,
                                QRectF(avail));
    }

    const QPointF drawPos = magnifierCenter(translated);
    const QRectF magniRect(0, 0, m_pixels, m_pixels);

    QRectF crossHairTop(drawPos.x() + magZoom * (-0.5),
                        drawPos.y() - magZoom * (m_magPixels + 0.5),
                        magZoom,
                        magZoom * (m_magPixels));
    QRectF crossHairRight(drawPos.x() + magZoom * (0.5),
                          drawPos.y() + magZoom * (-0.5),
                          magZoom * (m_magPixels),
                          magZoom);
    QRectF crossHairBottom(drawPos.x() + magZoom * (-0.5),
                           drawPos.y() + magZoom * (0.5),
                           magZoom,
                           magZoom * (m_magPixels));
    QRectF crossHairLeft(drawPos.x() - magZoom * (m_magPixels + 0.5),
                         drawPos.y() + magZoom * (-0.5),
                         magZoom * (m_magPixels),
                         magZoom);
    const auto frag =
      QPainter::PixmapFragment::create(drawPos, magniRect, magZoom, magZoom);

    painter.setRenderHint(QPainter::Antialiasing, true);
    QPainterPath path = QPainterPath();
    path.addEllipse(drawPos, m_pixels * magZoom / 2, m_pixels * magZoom / 2);
    painter.setClipPath(path);

    painter.drawPixmapFragments(&frag, 1, patch, QPainter::OpaqueHint);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    for (const auto& rect :
         { crossHairTop, crossHairRight, crossHairBottom, crossHairLeft }) {
        painter.fillRect(rect, m_color);
    }
    QPen pen(m_borderColor);
    pen.setWidth(4);
    painter.setPen(pen);
    painter.drawEllipse(
      drawPos, m_pixels * magZoom / 2, m_pixels * magZoom / 2);
}

// https://invent.kde.org/graphics/spectacle/-/blob/master/src/QuickEditor/QuickEditor.cpp#L841
void MagnifierWidget::drawMagnifier(QPainter& painter)
{
    const QPoint translated =
      m_hasCursorPos ? m_cursorPos : QWidget::mapFromGlobal(QCursor::pos());
    const qreal dpr = m_screenshot.devicePixelRatio();

    int magX = qRound(translated.x() * dpr) - m_magPixels;
    int offsetX = 0;
    if (magX < 0) {
        offsetX = magX;
        magX = 0;
    } else {
        const int maxX = m_screenshot.width() - m_pixels;
        if (magX > maxX) {
            offsetX = magX - maxX;
            magX = maxX;
        }
    }
    int magY = qRound(translated.y() * dpr) - m_magPixels;
    int offsetY = 0;
    if (magY < 0) {
        offsetY = magY;
        magY = 0;
    } else {
        const int maxY = m_screenshot.height() - m_pixels;
        if (magY > maxY) {
            offsetY = magY - maxY;
            magY = maxY;
        }
    }
    QRectF magniRect(magX, magY, m_pixels, m_pixels);

    const QPointF drawPos = magnifierCenter(translated);
    QRectF crossHairTop(drawPos.x() + magZoom * (offsetX - 0.5),
                        drawPos.y() - magZoom * (m_magPixels + 0.5),
                        magZoom,
                        magZoom * (m_magPixels + offsetY));
    QRectF crossHairRight(drawPos.x() + magZoom * (0.5 + offsetX),
                          drawPos.y() + magZoom * (offsetY - 0.5),
                          magZoom * (m_magPixels - offsetX),
                          magZoom);
    QRectF crossHairBottom(drawPos.x() + magZoom * (offsetX - 0.5),
                           drawPos.y() + magZoom * (0.5 + offsetY),
                           magZoom,
                           magZoom * (m_magPixels - offsetY));
    QRectF crossHairLeft(drawPos.x() - magZoom * (m_magPixels + 0.5),
                         drawPos.y() + magZoom * (offsetY - 0.5),
                         magZoom * (m_magPixels + offsetX),
                         magZoom);
    QRectF crossHairBorder(drawPos.x() - magZoom * (m_magPixels + 0.5) - 1,
                           drawPos.y() - magZoom * (m_magPixels + 0.5) - 1,
                           m_pixels * magZoom + 2,
                           m_pixels * magZoom + 2);
    const auto frag =
      QPainter::PixmapFragment::create(drawPos, magniRect, magZoom, magZoom);

    painter.fillRect(crossHairBorder, m_borderColor);
    painter.drawPixmapFragments(&frag, 1, m_screenshot, QPainter::OpaqueHint);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    for (const auto& rect :
         { crossHairTop, crossHairRight, crossHairBottom, crossHairLeft }) {
        painter.fillRect(rect, m_color);
    }
}
