// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

// This code is a modified version of the KDE software Spectacle
// /src/Gui/KSImageWidget.cpp commit cbbd6d45f6426ccbf1a82b15fdf98613ccccbbe9

#include "imagelabel.h"

ImageLabel::ImageLabel(QWidget* parent)
  : QLabel(parent)
  , m_pixmap(QPixmap())
{
    m_DSEffect = new QGraphicsDropShadowEffect(this);

    m_DSEffect->setBlurRadius(5);
    m_DSEffect->setOffset(0);
    m_DSEffect->setColor(QColor(Qt::black));

    setGraphicsEffect(m_DSEffect);
    setCursor(Qt::OpenHandCursor);
    setAlignment(Qt::AlignCenter);
    setMinimumSize(size());
}

void ImageLabel::setScreenshot(const QPixmap& pixmap)
{
    m_pixmap = pixmap;
    const QString tooltip =
      QStringLiteral("%1x%2 px").arg(m_pixmap.width()).arg(m_pixmap.height());
    setToolTip(tooltip);
    setScaledPixmap();
}

void ImageLabel::setScaledPixmap()
{
    const qreal scale = qApp->devicePixelRatio();
    QPixmap scaledPixmap = m_pixmap.scaled(
      size() * scale, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    scaledPixmap.setDevicePixelRatio(scale);
    setPixmap(scaledPixmap);
}

// drag handlers

void ImageLabel::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragStartPosition = event->pos();
        setCursor(Qt::ClosedHandCursor);
    }
}

void ImageLabel::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        setCursor(Qt::OpenHandCursor);
    }
}

void ImageLabel::mouseMoveEvent(QMouseEvent* event)
{
    if (!(event->buttons() & Qt::LeftButton)) {
        return;
    }
    if ((event->pos() - m_dragStartPosition).manhattanLength() <
        QGuiApplication::styleHints()->startDragDistance()) {
        return;
    }
    setCursor(Qt::OpenHandCursor);
    emit dragInitiated();
}

// resize handler
void ImageLabel::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event)
    setScaledPixmap();
}
