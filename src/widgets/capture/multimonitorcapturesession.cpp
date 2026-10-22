// SPDX-License-Identifier: GPL-3.0-or-later

#include "multimonitorcapturesession.h"

#include "utils/desktopinfo.h"
#include "utils/screengrabber.h"
#include "widgets/capture/capturewidget.h"

#include <QCoreApplication>
#include <QCursor>
#include <QGuiApplication>
#include <QScreen>
#include <QTimer>

namespace {
// Fast enough to feel immediate, cheap enough to be irrelevant. Only used
// where a global cursor position exists at all.
constexpr int POINTER_POLL_MS = 50;
}

MultiMonitorCaptureSession::MultiMonitorCaptureSession(QObject* parent)
  : QObject(parent)
{}

bool MultiMonitorCaptureSession::start(const CaptureRequest& request)
{
    const QList<QScreen*> screens = QGuiApplication::screens();
    if (screens.isEmpty()) {
        return false;
    }

    bool ok = true;
    ScreenGrabber grabber;
    const QPixmap fullDesktop = grabber.grabFullDesktop(ok);
    if (!ok || fullDesktop.isNull()) {
        return false;
    }

    m_geometries.clear();
    for (QScreen* screen : screens) {
        m_geometries.append(screen->geometry());
    }
    m_tracker = MonitorFocus::ActiveMonitorTracker(screens.size());

    {
        // Held only while the widgets are built, so each one crops from this
        // grab instead of calling the screenshot portal again.
        ScreenGrabber::SessionCache cache(fullDesktop);

        for (int i = 0; i < screens.size(); ++i) {
            CaptureRequest perScreen = request;
            perScreen.setSelectedMonitor(i);

            auto* widget = new CaptureWidget(perScreen);
            m_widgets.append(widget);

            connect(widget,
                    &CaptureWidget::pointerEnteredMonitor,
                    this,
                    &MultiMonitorCaptureSession::handlePointerEntered);
            connect(widget,
                    &CaptureWidget::editingStarted,
                    this,
                    &MultiMonitorCaptureSession::handleEditingStarted);
            connect(widget,
                    &QObject::destroyed,
                    this,
                    &MultiMonitorCaptureSession::handleWidgetDestroyed);
            widget->installEventFilter(this);

#if defined(Q_OS_WIN)
            widget->show();
#else
            widget->showFullScreen();
#endif
        }
    }

    // Nothing is armed until the pointer announces itself, so start every
    // display dimmed rather than showing N help overlays at once.
    for (auto& widget : m_widgets) {
        if (widget) {
            widget->setArmed(false);
        }
    }

    // Wayland has no global cursor position; there the per-widget Enter events
    // are the only signal, and polling would fight them.
    if (!DesktopInfo().waylandDetected()) {
        m_pollTimer = new QTimer(this);
        m_pollTimer->setInterval(POINTER_POLL_MS);
        connect(m_pollTimer, &QTimer::timeout, this, [this]() {
            handlePointerEntered(
              MonitorFocus::monitorIndexAt(m_geometries, QCursor::pos()));
        });
        m_pollTimer->start();
    }

    return true;
}

CaptureWidget* MultiMonitorCaptureSession::armedWidget() const
{
    const int active = m_tracker.activeMonitor();
    if (active < 0 || active >= m_widgets.size()) {
        return nullptr;
    }
    return m_widgets.at(active);
}

void MultiMonitorCaptureSession::handlePointerEntered(int monitorIndex)
{
    if (!m_tracker.pointerAt(monitorIndex)) {
        return;
    }
    applyArmedState();
    emit armedChanged(armedWidget());
}

bool MultiMonitorCaptureSession::eventFilter(QObject* watched, QEvent* event)
{
    // Keys that are not shortcuts land in whichever window has keyboard focus,
    // which need not be the armed one; hand them over until the user commits.
    if ((event->type() == QEvent::KeyPress ||
         event->type() == QEvent::KeyRelease) &&
        !m_tracker.isLatched()) {
        CaptureWidget* armed = armedWidget();
        if (armed && watched != armed) {
            QCoreApplication::sendEvent(armed, event);
            return true;
        }
    }
    return QObject::eventFilter(watched, event);
}

void MultiMonitorCaptureSession::handleEditingStarted(int monitorIndex)
{
    if (m_tracker.isLatched()) {
        return;
    }
    m_tracker.beginEditing(monitorIndex);

    if (m_pollTimer) {
        m_pollTimer->stop();
    }
    applyArmedState();
    discardAllExcept(m_tracker.activeMonitor());
    // The click that latched gave this window real keyboard focus, so it no
    // longer needs keys from elsewhere; application-wide shortcuts would now
    // only steal keys from other Flameshot windows, such as pins.
    if (CaptureWidget* kept = armedWidget()) {
        kept->restoreWindowShortcuts();
    }
    emit latched(armedWidget());
}

void MultiMonitorCaptureSession::applyArmedState()
{
    const int active = m_tracker.activeMonitor();
    for (int i = 0; i < m_widgets.size(); ++i) {
        if (m_widgets.at(i)) {
            m_widgets.at(i)->setArmed(i == active);
            // Until a display is armed the shortcuts keep their per-window
            // behaviour, so Esc still works from the focused one.
            if (active >= 0) {
                m_widgets.at(i)->setSharedShortcutsActive(i == active);
            }
        }
    }
}

void MultiMonitorCaptureSession::discardAllExcept(int keepIndex)
{
    m_tearingDown = true;
    for (int i = 0; i < m_widgets.size(); ++i) {
        CaptureWidget* widget = m_widgets.at(i);
        if (!widget || i == keepIndex) {
            continue;
        }
        // Without this each discarded widget would report a failed capture,
        // which main.cpp turns into an application exit.
        widget->discardSilently();
        widget->close();
    }
    m_tearingDown = false;
}

void MultiMonitorCaptureSession::handleWidgetDestroyed(QObject* dying)
{
    if (m_tearingDown) {
        return;
    }

    // A widget emits destroyed() from ~QWidget, before its QPointer is
    // cleared, so the dying one still looks alive here. It is past
    // ~CaptureWidget and must not be touched.
    int dyingIndex = -1;
    for (int i = 0; i < m_widgets.size(); ++i) {
        if (m_widgets.at(i) == dying) {
            dyingIndex = i;
            break;
        }
    }

    // A widget went away on its own: the user pressed Esc, or the capture
    // finished. Either way the session is over, so take the rest down quietly
    // -- the widget that closed has already reported the outcome.
    if (!m_tracker.isLatched()) {
        discardAllExcept(dyingIndex);
    }

    bool anyLeft = false;
    for (int i = 0; i < m_widgets.size(); ++i) {
        if (m_widgets.at(i) && i != dyingIndex) {
            anyLeft = true;
            break;
        }
    }
    if (!anyLeft) {
        deleteLater();
    }
}
