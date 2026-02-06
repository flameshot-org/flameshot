// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#include "x11shortcutfilter.h"
#include <X11/Xlib.h>
#include <X11/extensions/XInput2.h>
#include <X11/keysym.h>

// Worker: runs in a dedicated thread with its own X11 connection

X11ShortcutWorker::X11ShortcutWorker(QObject* parent)
  : QObject(parent)
  , m_running(false)
{}

void X11ShortcutWorker::stop()
{
    m_running = false;
}

void X11ShortcutWorker::run()
{
    Display* dpy = XOpenDisplay(nullptr);
    if (!dpy) {
        return;
    }
    int xiOpcode, eventBase, errorBase;
    if (!XQueryExtension(
          dpy, "XInputExtension", &xiOpcode, &eventBase, &errorBase)) {
        XCloseDisplay(dpy);
        return;
    }
    int printKeycode = XKeysymToKeycode(dpy, XK_Print);
    if (printKeycode == 0) {
        XCloseDisplay(dpy);
        return;
    }
    // Register for raw key events on root window
    unsigned char mask[XIMaskLen(XI_LASTEVENT)] = { 0 };
    XIEventMask evmask;
    evmask.deviceid = XIAllMasterDevices;
    evmask.mask_len = sizeof(mask);
    evmask.mask = mask;
    XISetMask(mask, XI_RawKeyPress);
    XISelectEvents(dpy, DefaultRootWindow(dpy), &evmask, 1);
    XFlush(dpy);
    m_running = true;
    while (m_running) {
        while (XPending(dpy) > 0) {
            XEvent xev;
            XNextEvent(dpy, &xev);
            if (xev.xcookie.type == GenericEvent &&
                xev.xcookie.extension == xiOpcode) {
                if (XGetEventData(dpy, &xev.xcookie)) {
                    if (xev.xcookie.evtype == XI_RawKeyPress) {
                        auto* rawEvent =
                          static_cast<XIRawEvent*>(xev.xcookie.data);
                        if (rawEvent->detail == printKeycode) {
                            emit printPressed();
                        }
                    }
                    XFreeEventData(dpy, &xev.xcookie);
                }
            }
        }
        // Small sleep to avoid busy waiting
        QThread::msleep(5);
    }
    XCloseDisplay(dpy);
}

// Filter: manages the worker thread

X11ShortcutFilter::X11ShortcutFilter(QObject* parent)
  : QObject(parent)
  , m_worker(new X11ShortcutWorker())
{
    m_worker->moveToThread(&m_thread);
    connect(&m_thread, &QThread::started, m_worker, &X11ShortcutWorker::run);
    connect(
      m_worker, &X11ShortcutWorker::printPressed,
      this, &X11ShortcutFilter::printPressed,
      Qt::QueuedConnection);
    connect(&m_thread, &QThread::finished, m_worker, &QObject::deleteLater);
    m_thread.start();
}

X11ShortcutFilter::~X11ShortcutFilter()
{
    m_worker->stop();
    m_thread.quit();
    m_thread.wait(2000);
}
