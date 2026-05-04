// ================= windowhighlightoverlay.cpp =================
#include "windowhighlightoverlay.h"
#include <QPainter>
#include <QGuiApplication>
#include <QScreen>
#include <QDebug>

#if defined( Q_OS_WIN )
#include <dwmapi.h>
#endif

#if defined( Q_OS_WIN )
#include <dwmapi.h>
#endif

static constexpr QColor BORDER_COLOR(136, 0, 170, 255);
static constexpr int    BORDER_WIDTH = 3;
#if defined( Q_OS_WIN )
WindowHighlightOverlay* WindowHighlightOverlay::s_instance_ = nullptr;

WindowHighlightOverlay::WindowHighlightOverlay(QWidget *parent)
  : QWidget(parent)
{
    //setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute( Qt::WA_NoSystemBackground );
    setAttribute(Qt::WA_TransparentForMouseEvents );
    setAttribute( Qt::WA_TranslucentBackground );
    setAttribute(Qt::WA_ShowWithoutActivating);

    setWindowFlag(Qt::FramelessWindowHint);
    setWindowFlag(Qt::Tool);
    setWindowFlag(Qt::WindowStaysOnTopHint);
   // setOverlayWindowMouseBlocking( this );
    //blockMouseEventsOn( this );
}

WindowHighlightOverlay::~WindowHighlightOverlay()
{
    stopTracking();
}

void WindowHighlightOverlay::initVirtualDesktop()
{
    QRect geo;
    for (QScreen* s : QGuiApplication::screens())
        geo |= s->geometry();
    setGeometry(geo);
}

void WindowHighlightOverlay::startTracking(int intervalMs)
{
    if (!isVisible()) show();
    raise();
    installMouseHook();
    connect(&timer_, &QTimer::timeout, this, &WindowHighlightOverlay::updateTarget, Qt::UniqueConnection);
    timer_.start(intervalMs);
}

void WindowHighlightOverlay::stopTracking()
{
    timer_.stop();
    uninstallMouseHook();
    hide();
}

void WindowHighlightOverlay::installMouseHook()
{
    if (!mouseHook_) {
        s_instance_ = this;
        mouseHook_ = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, nullptr, 0);
    }
}

void WindowHighlightOverlay::uninstallMouseHook()
{
    if (mouseHook_) {
        UnhookWindowsHookEx(mouseHook_);
        mouseHook_ = nullptr;
        s_instance_ = nullptr;
    }
}

LRESULT CALLBACK WindowHighlightOverlay::LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION && wParam == WM_LBUTTONDOWN && s_instance_) {
        const MSLLHOOKSTRUCT* info = reinterpret_cast<const MSLLHOOKSTRUCT*>(lParam);
        HWND hwnd = WindowFromPoint(info->pt);
        if (hwnd && hwnd != reinterpret_cast<HWND>(s_instance_->winId())) {
            s_instance_->selectedHwnd_ = hwnd;
            emit s_instance_->panelSelected(hwnd);
            QMetaObject::invokeMethod(s_instance_, "stopTracking", Qt::QueuedConnection);
        }
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}


void WindowHighlightOverlay::updateTarget()
{
    QRect r = getWindowUnderCursor();
    if (r != targetRect_) {
        targetRect_ = r;
        update();
    }
}

/*QRect WindowHighlightOverlay::getWindowUnderCursor() const
{
    POINT pt;
    GetCursorPos(&pt);
    HWND hwnd = WindowFromPoint(pt);
    if (!hwnd || hwnd == reinterpret_cast<HWND>(winId())) return {};
    RECT rc;
    if (!GetWindowRect(hwnd, &rc)) return {};
    return QRect(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
}*/

QRect WindowHighlightOverlay::getWindowUnderCursor() const
{
    POINT pt;
    GetCursorPos(&pt);

    HWND hwnd = WindowFromPoint(pt);
    if (!hwnd || hwnd == reinterpret_cast<HWND>(winId())) {
        return {};
    }

    RECT rc{};
    HRESULT hr = DwmGetWindowAttribute(
      hwnd,
      DWMWA_EXTENDED_FRAME_BOUNDS,
      &rc,
      sizeof(rc)
      );

    if (FAILED(hr)) {
        if (!GetWindowRect(hwnd, &rc)) {
            return {};
        }
    }

    QRect rect(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);

           // Optional fine adjustment to avoid clipping borders
    rect = rect.adjusted(1, 1, -1, -1);

    return rect;
}
#endif

void WindowHighlightOverlay::paintEvent(QPaintEvent *)
{
    if (!targetRect_.isValid()) return;

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

           // Nearly transparent color that still blocks mouse events (Qt::transparent does not)
    QColor maskBlocker(0, 0, 0, 1);
    p.fillRect(this->rect(), maskBlocker);

           // Draw border
    p.setPen(QPen(BORDER_COLOR, BORDER_WIDTH));
    const QPoint offset = -geometry().topLeft();
    QRect local = targetRect_.translated(offset);
    p.drawRect(local.adjusted(BORDER_WIDTH / 2, BORDER_WIDTH / 2,
                              -BORDER_WIDTH / 2, -BORDER_WIDTH / 2));
}

#if defined( Q_OS_WIN )
void WindowHighlightOverlay::setOverlayWindowMouseBlocking(QWidget* widget) {
    HWND hwnd = reinterpret_cast<HWND>(widget->winId());

    LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);

    exStyle &= ~WS_EX_TRANSPARENT;                 // Block click-through
    exStyle |= WS_EX_LAYERED | WS_EX_TOOLWINDOW;   // Floating layer, no taskbar icon

    SetWindowLong(hwnd, GWL_EXSTYLE, exStyle);

           // Optional: keep the window always on top
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
}

void WindowHighlightOverlay::blockMouseEventsOn(QWidget* widget) {
    HWND hwnd = reinterpret_cast<HWND>(widget->winId());

    LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);

    exStyle &= ~WS_EX_TRANSPARENT;                 // Block click-through
    exStyle |= WS_EX_LAYERED | WS_EX_TOOLWINDOW;   // No taskbar icon, can be made transparent

    SetWindowLong(hwnd, GWL_EXSTYLE, exStyle);

    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
}
#endif
