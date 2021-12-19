// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "pinwidget.h"
#include "qguiappcurrentscreen.h"
#include "src/utils/confighandler.h"
#include "src/utils/globalvalues.h"
#include <QApplication>
#include <QLabel>
#include <QScreen>
#include <QShortcut>
#include <QVBoxLayout>
#include <QWheelEvent>

PinWidget::PinWidget(const QPixmap& pixmap,
                     const QRect& geometry,
                     QWidget* parent)
  : QWidget(parent)
  , m_pixmap(pixmap)
{
    setWindowIcon(QIcon(GlobalValues::iconPath()));
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    // set the bottom widget background transparent
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);

    ConfigHandler conf;
    m_baseColor = conf.uiColor();
    m_hoverColor = conf.contrastUiColor();

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

    qreal devicePixelRatio = 1;
#if defined(Q_OS_MACOS)
    QScreen* currentScreen = QGuiAppCurrentScreen().currentScreen();
    if (currentScreen) {
        devicePixelRatio = currentScreen->devicePixelRatio();
    }
#endif
    const int m = margin * devicePixelRatio;
    QRect adjusted_pos = geometry + QMargins(m, m, m, m);
    setGeometry(adjusted_pos);
#if defined(Q_OS_MACOS)
    if (currentScreen) {
        QPoint topLeft = currentScreen->geometry().topLeft();
        adjusted_pos.setX((adjusted_pos.x() - topLeft.x()) / devicePixelRatio +
                          topLeft.x());

        adjusted_pos.setY((adjusted_pos.y() - topLeft.y()) / devicePixelRatio +
                          topLeft.y());
        adjusted_pos.setWidth(adjusted_pos.size().width() / devicePixelRatio);
        adjusted_pos.setHeight(adjusted_pos.size().height() / devicePixelRatio);
        resize(0, 0);
        move(adjusted_pos.x(), adjusted_pos.y());
    }
#endif
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
    ConfigHandler config;
    QPixmap scaledPixmap;

    const qreal scale = qApp->devicePixelRatio();

    if (config.antialiasingPinZoom()) {
        scaledPixmap = m_pixmap.scaled(
          size * scale, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    } else {
        scaledPixmap = m_pixmap.scaled(
          size * scale, Qt::KeepAspectRatio, Qt::FastTransformation);
    }

    scaledPixmap.setDevicePixelRatio(scale);
    m_label->setPixmap(scaledPixmap);
}
