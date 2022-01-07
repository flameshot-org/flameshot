#include "colorgrabwidget.h"
#include "sidepanelwidget.h"

#include "colorutils.h"
#include "confighandler.h"
#include "overlaymessage.h"
#include "src/core/qguiappcurrentscreen.h"
#include <QApplication>
#include <QDebug>
#include <QKeyEvent>
#include <QPainter>
#include <QScreen>
#include <QShortcut>
#include <QTimer>
#include <stdexcept>

// Width (= height) and zoom level of the widget before the user clicks
#define WIDTH1 77
#define ZOOM1 11
// Width (= height) and zoom level of the widget after the user clicks
#define WIDTH2 165
#define ZOOM2 15

// NOTE: WIDTH1(2) should be divisible by ZOOM1(2) for best precision.
//       WIDTH1 should be odd so the cursor can be centered on a pixel.

ColorGrabWidget::ColorGrabWidget(QPixmap* p, QWidget* parent)
  : QWidget(parent)
  , m_pixmap(p)
  , m_mousePressReceived(false)
  , m_extraZoomActive(false)
  , m_magnifierActive(false)
{
    if (p == nullptr) {
        throw std::logic_error("Pixmap must not be null");
    }
    setAttribute(Qt::WA_DeleteOnClose);
    // We don't need this widget to receive mouse events because we use
    // eventFilter on other objects that do
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_QuitOnClose, false);
    // On Windows: don't activate the widget so CaptureWidget remains active
    setAttribute(Qt::WA_ShowWithoutActivating);
    setWindowFlags(Qt::BypassWindowManagerHint | Qt::WindowStaysOnTopHint |
                   Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus);
    setMouseTracking(true);
}

void ColorGrabWidget::startGrabbing()
{
    // NOTE: grabMouse() would prevent move events being received
    // With this method we just need to make sure that mouse press and release
    // events get consumed before they reach their target widget.
    // This is undone in the destructor.
    qApp->setOverrideCursor(Qt::CrossCursor);
    qApp->installEventFilter(this);
    OverlayMessage::pushKeyMap(
      { { "Enter or Left Click", tr("Accept color") },
        { "Hold Left Click", tr("Precisely select color") },
        { "Space or Right Click", tr("Toggle magnifier") },
        { "Esc", tr("Cancel") } });
}

QColor ColorGrabWidget::color()
{
    return m_color;
}

bool ColorGrabWidget::eventFilter(QObject*, QEvent* event)
{
    // Consume shortcut events and handle key presses from whole app
    if (event->type() == QEvent::KeyPress ||
        event->type() == QEvent::Shortcut) {
        QKeySequence key = event->type() == QEvent::KeyPress
                             ? static_cast<QKeyEvent*>(event)->key()
                             : static_cast<QShortcutEvent*>(event)->key();
        if (key == Qt::Key_Escape) {
            emit grabAborted();
            finalize();
        } else if (key == Qt::Key_Return || key == Qt::Key_Enter) {
            emit colorGrabbed(m_color);
            finalize();
        } else if (key == Qt::Key_Space && !m_extraZoomActive) {
            setMagnifierActive(!m_magnifierActive);
        }
        return true;
    } else if (event->type() == QEvent::MouseMove) {
        // NOTE: This relies on the fact that CaptureWidget tracks mouse moves

        if (m_extraZoomActive && !geometry().contains(cursorPos())) {
            setExtraZoomActive(false);
            return true;
        }
        if (!m_extraZoomActive && !m_magnifierActive) {
            // This fixes an issue when the mouse leaves the zoom area before
            // the widget even appears.
            hide();
        }
        if (!m_extraZoomActive) {
            // Update only before the user clicks the mouse, after the mouse
            // press the widget remains static.
            updateWidget();
        }

        // Hide overlay message when cursor is over it
        OverlayMessage* overlayMsg = OverlayMessage::instance();
        overlayMsg->setVisibility(
          !overlayMsg->geometry().contains(cursorPos()));

        m_color = getColorAtPoint(cursorPos());
        emit colorUpdated(m_color);
        return true;
    } else if (event->type() == QEvent::MouseButtonPress) {
        m_mousePressReceived = true;
        auto* e = static_cast<QMouseEvent*>(event);
        if (e->buttons() == Qt::RightButton) {
            setMagnifierActive(!m_magnifierActive);
        } else if (e->buttons() == Qt::LeftButton) {
            setExtraZoomActive(true);
        }
        return true;
    } else if (event->type() == QEvent::MouseButtonRelease) {
        if (!m_mousePressReceived) {
            // Do not consume event if it corresponds to the mouse press that
            // triggered the color grabbing in the first place. This prevents
            // focus issues in the capture widget when the color grabber is
            // closed.
            return false;
        }
        auto* e = static_cast<QMouseEvent*>(event);
        if (e->button() == Qt::LeftButton && m_extraZoomActive) {
            emit colorGrabbed(getColorAtPoint(cursorPos()));
            finalize();
        }
        return true;
    } else if (event->type() == QEvent::MouseButtonDblClick) {
        return true;
    }
    return false;
}

void ColorGrabWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.drawImage(QRectF(0, 0, width(), height()), m_previewImage);
}

void ColorGrabWidget::showEvent(QShowEvent*)
{
    updateWidget();
}

QPoint ColorGrabWidget::cursorPos() const
{
    return QCursor::pos(QGuiAppCurrentScreen().currentScreen());
}

/// @note The point is in screen coordinates.
QColor ColorGrabWidget::getColorAtPoint(const QPoint& p) const
{
    if (m_extraZoomActive && geometry().contains(p)) {
        QPoint point = mapFromGlobal(p);
        // we divide coordinate-wise to avoid rounding to nearest
        return m_previewImage.pixel(
          QPoint(point.x() / ZOOM2, point.y() / ZOOM2));
    }
    QPoint point = p;
#if defined(Q_OS_MACOS)
    QScreen* currentScreen = QGuiAppCurrentScreen().currentScreen();
    if (currentScreen) {
        point = QPoint((p.x() - currentScreen->geometry().x()) *
                         currentScreen->devicePixelRatio(),
                       (p.y() - currentScreen->geometry().y()) *
                         currentScreen->devicePixelRatio());
    }
#endif
    QPixmap pixel = m_pixmap->copy(QRect(point, point));
    return pixel.toImage().pixel(0, 0);
}

void ColorGrabWidget::setExtraZoomActive(bool active)
{
    m_extraZoomActive = active;
    if (!active && !m_magnifierActive) {
        hide();
    } else {
        if (!isVisible()) {
            QTimer::singleShot(250, this, [this]() { show(); });
        } else {
            QTimer::singleShot(250, this, [this]() { updateWidget(); });
        }
    }
}

void ColorGrabWidget::setMagnifierActive(bool active)
{
    m_magnifierActive = active;
    setVisible(active);
}

void ColorGrabWidget::updateWidget()
{
    int width = m_extraZoomActive ? WIDTH2 : WIDTH1;
    float zoom = m_extraZoomActive ? ZOOM2 : ZOOM1;
    // Set window size and move its center to the mouse cursor
    QRect rect(0, 0, width, width);

    auto realCursorPos = cursorPos();
    auto adjustedCursorPos = realCursorPos;

#if defined(Q_OS_MACOS)
    QScreen* currentScreen = QGuiAppCurrentScreen().currentScreen();
    if (currentScreen) {
        adjustedCursorPos =
          QPoint((realCursorPos.x() - currentScreen->geometry().x()) *
                   currentScreen->devicePixelRatio(),
                 (realCursorPos.y() - currentScreen->geometry().y()) *
                   currentScreen->devicePixelRatio());
    }
#endif

    rect.moveCenter(cursorPos());
    setGeometry(rect);
    // Store a pixmap containing the zoomed-in section around the cursor
    QRect sourceRect(0, 0, width / zoom, width / zoom);
    sourceRect.moveCenter(adjustedCursorPos);
    m_previewImage = m_pixmap->copy(sourceRect).toImage();
    // Repaint
    update();
}

void ColorGrabWidget::finalize()
{
    qApp->removeEventFilter(this);
    qApp->restoreOverrideCursor();
    OverlayMessage::pop();
    close();
}
