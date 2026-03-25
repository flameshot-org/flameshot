// ================= windowhighlightoverlay.cpp =================
#include "windowhighlightoverlay.h"
#include <QPainter>
#include <QGuiApplication>
#include <QScreen>
#include <QDebug>
#include <dwmapi.h>

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

           // Ajuste fino opcional si aún lo ves un poco grande
    rect = rect.adjusted(1, 1, -1, -1);

    return rect;
}
#endif

void WindowHighlightOverlay::paintEvent(QPaintEvent *)
{
    if (!targetRect_.isValid()) return;

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

           // ⚠️ Usamos color casi transparente que BLOQUEA eventos
    QColor maskBlocker(0, 0, 0, 1); // un alfa mínimo, pero evita que el clic pase
    p.fillRect(this->rect(), maskBlocker); // ¡NO uses Qt::transparent!

           // Dibuja borde rojo (o cualquier color que uses)
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

    exStyle &= ~WS_EX_TRANSPARENT;                 // Bloquea el paso de clics
    exStyle |= WS_EX_LAYERED | WS_EX_TOOLWINDOW;   // Capa flotante y sin icono en la barra

    SetWindowLong(hwnd, GWL_EXSTYLE, exStyle);

           // Opcional: mantener la ventana siempre arriba
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
}

void WindowHighlightOverlay::blockMouseEventsOn(QWidget* widget) {
    HWND hwnd = reinterpret_cast<HWND>(widget->winId());

    LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);

    exStyle &= ~WS_EX_TRANSPARENT;                 // 🔒 Bloquea clics que pasen
    exStyle |= WS_EX_LAYERED | WS_EX_TOOLWINDOW;   // No aparece en la barra, se puede hacer transparente

    SetWindowLong(hwnd, GWL_EXSTYLE, exStyle);

    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
}
#endif
