// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#pragma once

#include <QList>
#include <QPoint>
#include <QRect>

#include <memory>

#if defined(Q_OS_WIN)
#include "touchpadscrollclassifier.h"
#endif

// Finds the windows and UI elements under the pointer using native platform
// APIs, so that a click can select them without dragging a rectangle.
class SmartRegionDetector
{
public:
    virtual ~SmartRegionDetector() = default;

    // Record the visible windows. Call this before the capture overlay is
    // shown, so the overlay itself is not part of the snapshot.
    virtual void snapshotWindows() = 0;

    // Regions under globalPosition, in Qt global logical coordinates, ordered
    // from the deepest UI element to the window that contains it. Returns an
    // empty list when there is no window under the position.
    virtual QList<QRect> regionsAt(const QPoint& globalPosition) = 0;

    // False when only whole windows can be detected, for example because the
    // macOS Accessibility permission has not been granted.
    virtual bool elementDetectionAvailable() const { return true; }

    // Whether this build and session have a detector backend.
    static bool isSupported();

    // Returns nullptr when isSupported() is false.
    static std::unique_ptr<SmartRegionDetector> create();
};

// Turns raw element geometries (deepest first) into the region chain returned
// by SmartRegionDetector::regionsAt(): regions are clipped to the window, and
// tiny, duplicate or unrelated ones are dropped. The window always ends the
// chain.
QList<QRect> buildRegionChain(const QList<QRect>& elementGeometries,
                              const QRect& windowGeometry,
                              const QPoint& globalPosition);

#if defined(Q_OS_WIN)
// Recognizes wheel messages from a precision touchpad that scrolls content
// opposite to the fingers, which is the Windows default. Qt does not report
// them as inverted wheel events, unlike on macOS, and Windows delivers them
// as ordinary mouse wheel messages. The touchpad's raw input, which arrives
// while fingers are on it, tells them apart from mouse wheel messages.
class TouchpadScrollTracker
{
public:
    TouchpadScrollTracker() = default;
    ~TouchpadScrollTracker();
    TouchpadScrollTracker(const TouchpadScrollTracker&) = delete;
    TouchpadScrollTracker& operator=(const TouchpadScrollTracker&) = delete;

    // Start receiving the touchpad's raw input. Until then, messages are
    // only written to the input log.
    void enable() { m_enabled = true; }

    // Pass every native message of the window that receives wheel input.
    // Returns whether the message may have changed the foreground window.
    bool handleMessage(const void* message);

    // Whether the last wheel message came from a touchpad.
    bool fromTouchpad() const { return m_fromTouchpad; }
    // Whether it came from a touchpad that scrolls content opposite to the
    // fingers.
    bool naturalScroll() const { return m_naturalScroll; }

private:
    void attach(void* window);

    TouchpadScrollClassifier m_classifier;
    void* m_window = nullptr;
    bool m_enabled = false;
    bool m_registered = false;
    bool m_naturalSetting = true;
    bool m_fromTouchpad = false;
    bool m_naturalScroll = false;
    quint32 m_lastInputTime = 0;
    int m_inputBurst = 0;
};
#endif

#if defined(Q_OS_MACOS)
// UI element detection on macOS needs the Accessibility permission.
bool macOsAccessibilityTrusted();
void requestMacOsAccessibilityPermission();
#endif
