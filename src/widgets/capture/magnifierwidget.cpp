// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "magnifierwidget.h"

#include <QPainter>
#include <QPainterPath>
#include <QtMath>

MagnifierWidget::MagnifierWidget(const QPixmap& p,
                                 const QColor& c,
                                 bool isSquare,
                                 QWidget* parent)
  : QWidget(parent)
  , m_devicePixelRatio(p.devicePixelRatio())
  , m_square(isSquare)
  , m_color(c)
  , m_borderColor(c)
  , m_screenshot(p)
{
    setFixedSize(m_pixels * magZoom + 4, m_pixels * magZoom + 4);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    m_color.setAlpha(130);
    // Source coordinates below are physical pixels, including on HiDPI screens.
    m_screenshot.setDevicePixelRatio(1);
    setCursorPosition(parent->mapFromGlobal(QCursor::pos()));
}

void MagnifierWidget::setCursorPosition(const QPoint& position)
{
    m_cursorPosition = position;
    QPoint lens = position + QPoint(m_magOffset, m_magOffset);
    if (lens.x() + width() > parentWidget()->width()) {
        lens.setX(position.x() - m_magOffset - width());
    }
    if (lens.y() + height() > parentWidget()->height()) {
        lens.setY(position.y() - m_magOffset - height());
    }
    lens.setX(qBound(0, lens.x(), qMax(0, parentWidget()->width() - width())));
    lens.setY(
      qBound(0, lens.y(), qMax(0, parentWidget()->height() - height())));
    move(lens);
    update();
}

void MagnifierWidget::paintEvent(QPaintEvent*)
{
    const QPoint pixel(qFloor(m_cursorPosition.x() * m_devicePixelRatio),
                       qFloor(m_cursorPosition.y() * m_devicePixelRatio));
    QPoint source = pixel - QPoint(m_magPixels, m_magPixels);
    if (m_square) {
        source.setX(
          qBound(0, source.x(), qMax(0, m_screenshot.width() - m_pixels)));
        source.setY(
          qBound(0, source.y(), qMax(0, m_screenshot.height() - m_pixels)));
    }
    // Only allocate the tiny sample. Circular lenses use black padding at the
    // screen edge, without keeping a padded copy of the entire screenshot.
    QPixmap sample(m_pixels, m_pixels);
    sample.fill(Qt::black);
    {
        QPainter samplePainter(&sample);
        samplePainter.drawPixmap(-source, m_screenshot);
    }

    QPainter painter(this);
    const QRect lens(2, 2, m_pixels * magZoom, m_pixels * magZoom);
    painter.setRenderHint(QPainter::Antialiasing);
    if (!m_square) {
        QPainterPath clip;
        clip.addEllipse(QRectF(lens));
        painter.setClipPath(clip);
    }
    painter.drawPixmap(lens, sample);
    const QPoint cross = (pixel - source) * magZoom + lens.topLeft();
    // Leave the pixel under the cursor unobscured.
    const QRegion crosshair =
      (QRegion(QRect(cross.x(), lens.top(), magZoom, lens.height())) |
       QRegion(QRect(lens.left(), cross.y(), lens.width(), magZoom)))
        .subtracted(QRect(cross, QSize(magZoom, magZoom)));
    painter.save();
    painter.setClipRegion(crosshair, Qt::IntersectClip);
    painter.fillRect(lens, m_color);
    painter.restore();
    painter.setPen(QPen(m_borderColor, m_square ? 2 : 4));
    painter.setBrush(Qt::NoBrush);
    if (m_square) {
        painter.drawRect(lens);
    } else {
        painter.drawEllipse(lens);
    }
}
