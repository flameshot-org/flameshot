#include "colorgrabwidget.h"
#include "sidepanelwidget.h"

#include "src/core/qguiappcurrentscreen.h"
#include <QApplication>
#include <QKeyEvent>
#include <QPainter>
#include <QScreen>
#include <QShortcut>
#include <QTimer>
#include <stdexcept>

// Width and height are the same
#define WIDTH 165
// NOTE: WIDTH should be divisible by ZOOM_LEVEL for best precision
#define ZOOM_LEVEL 15

ColorGrabWidget::ColorGrabWidget(QPixmap* p, QWidget* parent)
  : QWidget(parent)
  , m_pixmap(p)
  , m_mousePressReceived(false)
{
    if (p == nullptr) {
        throw std::logic_error("Pixmap must not be null");
    }
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::BypassWindowManagerHint | Qt::FramelessWindowHint);
    setMouseTracking(true);
}

void ColorGrabWidget::startGrabbing()
{
    // NOTE: grabbing the mouse would prevent move events being received
    // With this method we just need to make sure that mouse press and release
    // events get consumed before they reach their target widget.
    // This is undone in the destructor.
    qApp->setOverrideCursor(Qt::CrossCursor);
    qApp->installEventFilter(this);
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
        }
        return true;
    } else if (event->type() == QEvent::MouseMove) {
        // NOTE: This relies on the fact that CaptureWidget tracks mouse moves
        m_color = getColorAtPoint(cursorPos());
        emit colorUpdated(m_color);
        return true;
    } else if (event->type() == QEvent::MouseButtonPress) {
        m_mousePressReceived = true;
        auto* e = static_cast<QMouseEvent*>(event);
        if (e->buttons() == Qt::LeftButton && !isVisible()) {
            QTimer::singleShot(500, this, [this]() { show(); });
        } else if (e->buttons() == Qt::RightButton) {
            show();
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
        if (e->button() == Qt::LeftButton) {
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
    // Set the window geometry
    QRect rect(0, 0, WIDTH, WIDTH);
    rect.moveCenter(cursorPos());
    setGeometry(rect);
    // Store a pixmap containing the zoomed-in section
    QRect sourceRect(
      0, 0, rect.width() / ZOOM_LEVEL, rect.width() / ZOOM_LEVEL);
    sourceRect.moveCenter(rect.center());
    m_previewImage = m_pixmap->copy(sourceRect).toImage();
}

QPoint ColorGrabWidget::cursorPos() const
{
    return QCursor::pos(QGuiAppCurrentScreen().currentScreen());
}

/// @note The point is in screen coordinates.
QColor ColorGrabWidget::getColorAtPoint(const QPoint& p) const
{
    if (isVisible() && geometry().contains(p)) {
        QPoint point = mapFromGlobal(p);
        // we divide coordinate-wise to avoid rounding to nearest
        return m_previewImage.pixel(
          QPoint(point.x() / ZOOM_LEVEL, point.y() / ZOOM_LEVEL));
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

void ColorGrabWidget::finalize()
{
    qApp->removeEventFilter(this);
    qApp->restoreOverrideCursor();
    close();
}
