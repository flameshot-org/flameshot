// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include <QGraphicsDropShadowEffect>
#include <QPinchGesture>

#include "pinwidget.h"
#include "qguiappcurrentscreen.h"
#include "src/utils/confighandler.h"
#include "src/utils/globalvalues.h"

#include <QLabel>
#include <QScreen>
#include <QShortcut>
#include <QVBoxLayout>
#include <QWheelEvent>

namespace
{
    static constexpr int MARGIN = 7;
    static constexpr int BLUR_RADIUS =  2 * MARGIN;
}

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
    m_layout->setContentsMargins(MARGIN, MARGIN, MARGIN, MARGIN);

    m_shadowEffect = new QGraphicsDropShadowEffect(this);
    m_shadowEffect->setColor(m_baseColor);
    m_shadowEffect->setBlurRadius(BLUR_RADIUS);
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
    const int m = MARGIN * devicePixelRatio;
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
    grabGesture(Qt::PinchGesture);
}

bool PinWidget::scrollEvent(QWheelEvent* event)
{
    const auto phase = event->phase();
    if (phase == Qt::ScrollPhase::ScrollUpdate) {
        const auto angle = event->angleDelta();
        if(angle.y() == 0)
        {
            return true;
        }
        currentStepScaleFactor = angle.y() > 0 ? currentStepScaleFactor + 0.03 : currentStepScaleFactor - 0.03;
        m_expanding = currentStepScaleFactor > 1.0;
    }
    if (phase == Qt::ScrollPhase::ScrollEnd) {
        scaleFactor *= currentStepScaleFactor;
        currentStepScaleFactor = 1;
        m_expanding = false;
    }
    update();
    return true;
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
    scaleFactor = 1;
    currentStepScaleFactor = 1;
    update();
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
    const int offsetW = width() * m_offsetX;
    const int offsetH = height() * m_offsetY;
    move(m_dragStart.x() + delta.x() - offsetW,
         m_dragStart.y() + delta.y() - offsetH);
}

bool PinWidget::gestureEvent(QGestureEvent *event)
{
    if (QGesture *pinch = event->gesture(Qt::PinchGesture))
    {
        pinchTriggered(static_cast<QPinchGesture *>(pinch));
    }
    return true;
}

bool PinWidget::event(QEvent *event)
{
    if (event->type() == QEvent::Gesture)
    {
        return gestureEvent(static_cast<QGestureEvent*>(event));
    }
    else if(event->type() == QEvent::Wheel)
    {
        return scrollEvent(static_cast<QWheelEvent*>(event));
    }
    return QWidget::event(event);
}

void PinWidget::paintEvent(QPaintEvent * event)
{
    const auto aspectRatio =
      m_expanding ? Qt::KeepAspectRatioByExpanding : Qt::KeepAspectRatio;
    const auto transformType = ConfigHandler().antialiasingPinZoom()
                                 ? Qt::SmoothTransformation
                                 : Qt::FastTransformation;
    const qreal iw = m_pixmap.width();
    const qreal ih = m_pixmap.height();
    const QPixmap pix = m_pixmap.scaled(iw * currentStepScaleFactor * scaleFactor,
                    ih * currentStepScaleFactor * scaleFactor,
                    aspectRatio,
                    transformType);
    m_label->setPixmap(pix);
    adjustSize();
}

void PinWidget::pinchTriggered(QPinchGesture *gesture)
{
    const QPinchGesture::ChangeFlags changeFlags = gesture->changeFlags();
    if (changeFlags & QPinchGesture::ScaleFactorChanged) {
        currentStepScaleFactor = gesture->totalScaleFactor();
        m_expanding = currentStepScaleFactor > gesture->lastScaleFactor();
    }
    if (gesture->state() == Qt::GestureFinished) {
        scaleFactor *= currentStepScaleFactor;
        currentStepScaleFactor = 1;
        m_expanding = false;
    }
    update();
}

