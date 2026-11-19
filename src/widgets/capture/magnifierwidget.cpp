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

MagnifierWidget::MagnifierWidget(const QPixmap& p,
                                 const QColor& c,
                                 bool isSquare,
                                 QWidget* parent)
  : QWidget(parent)
  , m_devicePixelRatio(p.devicePixelRatio() > 0.0 ? p.devicePixelRatio() : 1.0)
  , m_square(isSquare)
  , m_color(c)
  , m_borderColor(c)
  , m_screenshot(p)
{
    setFixedSize(parent->width(), parent->height());
    setAttribute(Qt::WA_TransparentForMouseEvents);
    m_color.setAlpha(130);
    m_screenshot.setDevicePixelRatio(1.0);
    // add padding for circular magnifier
    QImage padded(m_screenshot.width() + 2 * m_magPixels,
                  m_screenshot.height() + 2 * m_magPixels,
                  QImage::Format_ARGB32);
    padded.fill(Qt::black);
    QPainter painter(&padded);
    painter.drawPixmap(m_magPixels, m_magPixels, m_screenshot);
    m_paddedScreenshot.convertFromImage(padded);
}

void MagnifierWidget::setCursorPos(const QPoint& pos)
{
    m_cursorPos = pos;
    m_hasCursorPos = true;
}

void MagnifierWidget::renderAt(QPainter& painter,
                               const QPoint& cursorPos,
                               const QRect& bounds)
{
    painter.save();
    if (m_square) {
        drawMagnifier(painter, cursorPos, bounds);
    } else {
        drawMagnifierCircle(painter, cursorPos, bounds);
    }
    painter.restore();
}

void MagnifierWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    if (!p.isActive()) {
        return;
    }
    const QPoint pos =
      m_hasCursorPos ? m_cursorPos : QWidget::mapFromGlobal(QCursor::pos());
    renderAt(p, pos, rect());
}

void MagnifierWidget::drawMagnifierCircle(QPainter& painter,
                                          const QPoint& cursorPos,
                                          const QRect& bounds)
{
    auto x = cursorPos.x() + m_magPixels;
    auto y = cursorPos.y() + m_magPixels;

    int magX = qBound(0,
                      qRound(cursorPos.x() * m_devicePixelRatio),
                      qMax(0, m_paddedScreenshot.width() - m_pixels));
    int magY = qBound(0,
                      qRound(cursorPos.y() * m_devicePixelRatio),
                      qMax(0, m_paddedScreenshot.height() - m_pixels));
    QRectF magniRect(magX, magY, m_pixels, m_pixels);

    qreal drawPosX = x + m_magOffset + m_pixels * magZoom / 2;
    if (drawPosX > bounds.right() - m_pixels * magZoom / 2) {
        drawPosX = x - m_magOffset - m_pixels * magZoom / 2;
    }
    qreal drawPosY = y + m_magOffset + m_pixels * magZoom / 2;
    if (drawPosY > bounds.bottom() - m_pixels * magZoom / 2) {
        drawPosY = y - m_magOffset - m_pixels * magZoom / 2;
    }
    QPointF drawPos(drawPosX, drawPosY);
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

    painter.drawPixmapFragments(
      &frag, 1, m_paddedScreenshot, QPainter::OpaqueHint);
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
void MagnifierWidget::drawMagnifier(QPainter& painter,
                                    const QPoint& cursorPos,
                                    const QRect& bounds)
{
    auto x = cursorPos.x();
    auto y = cursorPos.y();

    int magX = qRound(x * m_devicePixelRatio) - m_magPixels;
    int offsetX = 0;
    if (magX < 0) {
        offsetX = magX;
        magX = 0;
    } else {
        const int maxX = qMax(0, m_screenshot.width() - m_pixels);
        if (magX > maxX) {
            offsetX = magX - maxX;
            magX = maxX;
        }
    }
    int magY = qRound(y * m_devicePixelRatio) - m_magPixels;
    int offsetY = 0;
    if (magY < 0) {
        offsetY = magY;
        magY = 0;
    } else {
        const int maxY = qMax(0, m_screenshot.height() - m_pixels);
        if (magY > maxY) {
            offsetY = magY - maxY;
            magY = maxY;
        }
    }
    QRectF magniRect(magX, magY, m_pixels, m_pixels);

    qreal drawPosX = x + m_magOffset + m_pixels * magZoom / 2;
    if (drawPosX > bounds.right() - m_pixels * magZoom / 2) {
        drawPosX = x - m_magOffset - m_pixels * magZoom / 2;
    }
    qreal drawPosY = y + m_magOffset + m_pixels * magZoom / 2;
    if (drawPosY > bounds.bottom() - m_pixels * magZoom / 2) {
        drawPosY = y - m_magOffset - m_pixels * magZoom / 2;
    }
    QPointF drawPos(drawPosX, drawPosY);
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
