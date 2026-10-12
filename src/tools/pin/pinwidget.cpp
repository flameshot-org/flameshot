// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "pinwidget.h"
#include "core/qguiappcurrentscreen.h"
#include "utils/confighandler.h"
#include "utils/globalvalues.h"
#include "utils/screenshotsaver.h"

#include <QGraphicsDropShadowEffect>
#include <QGraphicsOpacityEffect>
#include <QLabel>
#include <QMenu>
#include <QPinchGesture>
#include <QScreen>
#include <QShortcut>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <QWindow>

namespace {
constexpr int MARGIN = 7;
constexpr int BLUR_RADIUS = 2 * MARGIN;
constexpr qreal STEP = 0.03;
constexpr qreal MIN_SIZE = 100.0;
constexpr int ZOOM_SETTLE_INTERVAL = 150;
}

PinWidget::PinWidget(const QPixmap& pixmap,
                     const QRect& geometry,
                     QWidget* parent)
  : QWidget(parent)
  , m_originalPixmap(pixmap)
  , m_pixmap(pixmap)
  , m_displaySize(pixmap.deviceIndependentSize().toSize())
  , m_layout(new QVBoxLayout(this))
  , m_label(new QLabel())
  , m_shadowEffect(new QGraphicsDropShadowEffect(this))
{
    setWindowIcon(QIcon(GlobalValues::iconPath()));
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint |
                   Qt::Dialog);
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle("flameshot-pin");
    ConfigHandler conf;
    m_baseColor = conf.uiColor();
    m_hoverColor = conf.contrastUiColor();

    // Coalesce zoom events and update geometry before the next paint.
    m_updateTimer.setSingleShot(true);
    connect(&m_updateTimer, &QTimer::timeout, this, &PinWidget::updatePixmap);
    m_qualityTimer.setSingleShot(true);
    m_qualityTimer.setInterval(ZOOM_SETTLE_INTERVAL);
    connect(&m_qualityTimer, &QTimer::timeout, this, &PinWidget::finishZoom);

    m_layout->setContentsMargins(MARGIN, MARGIN, MARGIN, MARGIN);

    m_shadowEffect->setColor(m_baseColor);
    m_shadowEffect->setBlurRadius(BLUR_RADIUS);
    m_shadowEffect->setOffset(0, 0);
    setGraphicsEffect(m_shadowEffect);
    setWindowOpacity(m_opacity);

    m_label->setPixmap(m_pixmap);
    m_layout->addWidget(m_label);

    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q), this, SLOT(close()));
    new QShortcut(Qt::Key_Escape, this, SLOT(close()));

    qreal devicePixelRatio = 1;
    QScreen* currentScreen = QGuiAppCurrentScreen().currentScreen();
    if (currentScreen != nullptr) {
        devicePixelRatio = currentScreen->devicePixelRatio();
    }

    const int margin =
      static_cast<int>(static_cast<double>(MARGIN) * devicePixelRatio);
    QRect adjusted_pos = geometry + QMargins(margin, margin, margin, margin);
    setGeometry(adjusted_pos);

    if (currentScreen != nullptr) {
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

    grabGesture(Qt::PinchGesture);

    this->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this,
            &QWidget::customContextMenuRequested,
            this,
            &PinWidget::showContextMenu);
}

void PinWidget::closePin()
{
    update();
    close();
}
bool PinWidget::scrollEvent(QWheelEvent* event)
{
    const auto phase = event->phase();
    if (phase == Qt::ScrollPhase::ScrollUpdate
#if defined(Q_OS_UNIX) || defined(Q_OS_WINDOWS)
        || phase == Qt::ScrollPhase::NoScrollPhase
#endif
    ) {
        const auto angle = event->angleDelta();
        if (angle.y() == 0) {
            return true;
        }
        m_currentStepScaleFactor = angle.y() > 0
                                     ? m_currentStepScaleFactor + STEP
                                     : m_currentStepScaleFactor - STEP;
        m_expanding = m_currentStepScaleFactor >= 1.0;
    }
#if defined(Q_OS_MACOS)
    // ScrollEnd is currently supported only on Mac OSX
    if (phase == Qt::ScrollPhase::ScrollEnd) {
#else
    else {
#endif
        m_scaleFactor *= m_currentStepScaleFactor;
        m_currentStepScaleFactor = 1.0;
        m_expanding = false;
    }

    m_qualityTimer.stop();
    m_updateTimer.start();
    return true;
}

void PinWidget::enterEvent(QEnterEvent*)
{
    m_shadowEffect->setColor(m_hoverColor);
}

void PinWidget::leaveEvent(QEvent*)
{
    m_shadowEffect->setColor(m_baseColor);
}

void PinWidget::mouseDoubleClickEvent(QMouseEvent*)
{
    closePin();
}

void PinWidget::mousePressEvent(QMouseEvent* e)
{
    if (QWindow* window = windowHandle(); window != nullptr) {
        window->startSystemMove();
        return;
    }
}

void PinWidget::mouseMoveEvent(QMouseEvent* e) {}

void PinWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_0) {
        m_opacity = 1.0;
    } else if (event->key() == Qt::Key_9) {
        m_opacity = 0.9;
    } else if (event->key() == Qt::Key_8) {
        m_opacity = 0.8;
    } else if (event->key() == Qt::Key_7) {
        m_opacity = 0.7;
    } else if (event->key() == Qt::Key_6) {
        m_opacity = 0.6;
    } else if (event->key() == Qt::Key_5) {
        m_opacity = 0.5;
    } else if (event->key() == Qt::Key_4) {
        m_opacity = 0.4;
    } else if (event->key() == Qt::Key_3) {
        m_opacity = 0.3;
    } else if (event->key() == Qt::Key_2) {
        m_opacity = 0.2;
    } else if (event->key() == Qt::Key_1) {
        m_opacity = 0.1;
    }

    setWindowOpacity(m_opacity);
}
bool PinWidget::gestureEvent(QGestureEvent* event)
{
    if (QGesture* pinch = event->gesture(Qt::PinchGesture)) {
        pinchTriggered(static_cast<QPinchGesture*>(pinch));
    }
    return true;
}

void PinWidget::rotateLeft()
{
    m_rotateFactor = (m_rotateFactor + 3) % 4;
    m_pixmap =
      m_originalPixmap.transformed(QTransform().rotate(90 * m_rotateFactor));
    m_qualityTimer.stop();
    m_updateTimer.start();
}

void PinWidget::rotateRight()
{
    m_rotateFactor = (m_rotateFactor + 1) % 4;
    m_pixmap =
      m_originalPixmap.transformed(QTransform().rotate(90 * m_rotateFactor));
    m_qualityTimer.stop();
    m_updateTimer.start();
}

void PinWidget::increaseOpacity()
{
    m_opacity += 0.1;
    if (m_opacity > 1.0) {
        m_opacity = 1.0;
    }
    setWindowOpacity(m_opacity);
}

void PinWidget::decreaseOpacity()
{
    m_opacity -= 0.1;
    if (m_opacity < 0.0) {
        m_opacity = 0.0;
    }

    setWindowOpacity(m_opacity);
}

bool PinWidget::event(QEvent* event)
{
    if (event->type() == QEvent::Gesture) {
        return gestureEvent(static_cast<QGestureEvent*>(event));
    } else if (event->type() == QEvent::Wheel) {
        return scrollEvent(static_cast<QWheelEvent*>(event));
    }
    // A Wayland window can have a different DPR from its QScreen. Refresh
    // the pixels after a screen/scale change without changing logical geometry.
    if (event->type() == QEvent::ScreenChangeInternal
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
        || event->type() == QEvent::DevicePixelRatioChange
#endif
    ) {
        m_qualityTimer.start();
    }
    return QWidget::event(event);
}

void PinWidget::updatePixmap()
{
    const auto aspectRatio =
      m_expanding ? Qt::KeepAspectRatioByExpanding : Qt::KeepAspectRatio;
    const auto transformType = ConfigHandler().antialiasingPinZoom()
                                 ? Qt::SmoothTransformation
                                 : Qt::FastTransformation;
    const qreal iw = m_pixmap.width();
    const qreal ih = m_pixmap.height();
    const qreal nw = qBound(MIN_SIZE,
                            iw * m_currentStepScaleFactor * m_scaleFactor,
                            static_cast<qreal>(maximumWidth()));
    const qreal nh = qBound(MIN_SIZE,
                            ih * m_currentStepScaleFactor * m_scaleFactor,
                            static_cast<qreal>(maximumHeight()));

    const QSize scaledSize = m_pixmap.size().scaled(nw, nh, aspectRatio);
    m_displaySize = (QSizeF(scaledSize) / m_pixmap.devicePixelRatio()).toSize();
    renderPixmap(transformType);
    adjustSize();
    m_qualityTimer.start();
}

void PinWidget::renderPixmap(Qt::TransformationMode mode)
{
    // Scale the full-resolution source directly to the window's physical
    // pixels. Retaining the source DPR here would make QLabel resample the
    // result again.
    const qreal dpr = m_label->devicePixelRatioF();
    QPixmap pix =
      m_pixmap.scaled(m_displaySize * dpr, Qt::IgnoreAspectRatio, mode);
    pix.setDevicePixelRatio(dpr);
    m_label->setPixmap(pix);
}

void PinWidget::finishZoom()
{
    if (m_updateTimer.isActive()) {
        return;
    }
    // Only replace the pixels: the settled image must not resize the window.
    renderPixmap(Qt::SmoothTransformation);
}

void PinWidget::pinchTriggered(QPinchGesture* gesture)
{
    const QPinchGesture::ChangeFlags changeFlags = gesture->changeFlags();
    if (changeFlags & QPinchGesture::ScaleFactorChanged) {
        m_currentStepScaleFactor = gesture->totalScaleFactor();
        m_expanding = m_currentStepScaleFactor > gesture->lastScaleFactor();
    }
    if (gesture->state() == Qt::GestureFinished) {
        m_scaleFactor *= m_currentStepScaleFactor;
        m_currentStepScaleFactor = 1;
        m_expanding = false;
    }
    m_qualityTimer.stop();
    m_updateTimer.start();
}

void PinWidget::showContextMenu(const QPoint& pos)
{
    QMenu contextMenu(tr("Context menu"), this);

    QAction copyToClipboardAction(tr("Copy to clipboard"), this);
    connect(&copyToClipboardAction,
            &QAction::triggered,
            this,
            &PinWidget::copyToClipboard);
    contextMenu.addAction(&copyToClipboardAction);

    QAction saveToFileAction(tr("Save to file"), this);
    connect(
      &saveToFileAction, &QAction::triggered, this, &PinWidget::saveToFile);
    contextMenu.addAction(&saveToFileAction);

    contextMenu.addSeparator();

    QAction rotateRightAction(tr("Rotate Right"), this);
    connect(
      &rotateRightAction, &QAction::triggered, this, &PinWidget::rotateRight);
    contextMenu.addAction(&rotateRightAction);

    QAction rotateLeftAction(tr("Rotate Left"), this);
    connect(
      &rotateLeftAction, &QAction::triggered, this, &PinWidget::rotateLeft);
    contextMenu.addAction(&rotateLeftAction);

    QAction increaseOpacityAction(tr("Increase Opacity"), this);
    connect(&increaseOpacityAction,
            &QAction::triggered,
            this,
            &PinWidget::increaseOpacity);
    contextMenu.addAction(&increaseOpacityAction);

    QAction decreaseOpacityAction(tr("Decrease Opacity"), this);
    connect(&decreaseOpacityAction,
            &QAction::triggered,
            this,
            &PinWidget::decreaseOpacity);
    contextMenu.addAction(&decreaseOpacityAction);

    QAction closePinAction(tr("Close"), this);
    connect(&closePinAction, &QAction::triggered, this, &PinWidget::closePin);
    contextMenu.addSeparator();
    contextMenu.addAction(&closePinAction);

    contextMenu.exec(mapToGlobal(pos));
}

void PinWidget::copyToClipboard()
{
    saveToClipboard(m_pixmap);
}
void PinWidget::saveToFile()
{
    hide();
    saveToFilesystemGUI(m_pixmap);
    show();
}
