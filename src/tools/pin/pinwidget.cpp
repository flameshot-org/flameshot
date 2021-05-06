// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "pinwidget.h"
#include "src/utils/confighandler.h"
#include <QApplication>
#include <QLabel>
#include <QShortcut>
#include <QVBoxLayout>
#include <QWheelEvent>

PinWidget::PinWidget(const QPixmap& pixmap, QWidget* parent)
  : QWidget(parent)
  , m_pixmap(pixmap)
{
    setWindowIcon(QIcon(":img/app/flameshot.svg"));
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    // set the bottom widget background transparent
    setAttribute(Qt::WA_TranslucentBackground);

    ConfigHandler conf;
    m_baseColor = conf.uiMainColorValue();
    m_hoverColor = conf.uiContrastColorValue();

    m_layout = new QVBoxLayout(this);
    const int margin = this->margin();
    m_layout->setContentsMargins(margin, margin, margin, margin);

    m_shadowEffect = new QGraphicsDropShadowEffect(this);
    m_shadowEffect->setColor(m_baseColor);
    m_shadowEffect->setBlurRadius(2 * margin);
    m_shadowEffect->setOffset(0, 0);
    setGraphicsEffect(m_shadowEffect);

    m_label = new QLabel();
    m_label->setPixmap(m_pixmap);
    m_layout->addWidget(m_label);

    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q), this, SLOT(close()));
    new QShortcut(Qt::Key_Escape, this, SLOT(close()));
}

int PinWidget::margin() const
{
    return 7;
}

void PinWidget::wheelEvent(QWheelEvent* e)
{
    int val = e->angleDelta().y() > 0 ? 15 : -15;
    int newWidth = qBound(50, m_label->width() + val, maximumWidth());
    int newHeight = qBound(50, m_label->height() + val, maximumHeight());

    QSize size(newWidth, newHeight);
    setScaledPixmap(size);
    adjustSize();

    e->accept();
}

void PinWidget::enterEvent(QEvent*)
{
    m_shadowEffect->setColor(m_hoverColor);
}
void PinWidget::leaveEvent(QEvent*)
{
    m_shadowEffect->setColor(m_baseColor);
}

void PinWidget::mouseDoubleClickEvent(QMouseEvent*)
{
    close();
}

void PinWidget::mousePressEvent(QMouseEvent* e)
{
    m_dragStart = e->globalPos();
    m_offsetX = e->localPos().x() / width();
    m_offsetY = e->localPos().y() / height();
}

void PinWidget::mouseMoveEvent(QMouseEvent* e)
{
    const QPoint delta = e->globalPos() - m_dragStart;
    int offsetW = width() * m_offsetX;
    int offsetH = height() * m_offsetY;
    move(m_dragStart.x() + delta.x() - offsetW,
         m_dragStart.y() + delta.y() - offsetH);
}

void PinWidget::setScaledPixmap(const QSize& size)
{
    const qreal scale = qApp->devicePixelRatio();
    QPixmap scaledPixmap = m_pixmap.scaled(
      size * scale, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    scaledPixmap.setDevicePixelRatio(scale);
    m_label->setPixmap(scaledPixmap);
}
