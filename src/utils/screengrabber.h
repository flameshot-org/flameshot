// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include "utils/desktopinfo.h"

#include <QEvent>
#include <QList>
#include <QObject>
#include <QPixmap>
#include <QScreen>

class QEventLoop;
class QWidget;
class MonitorPreview;

class ScreenGrabber : public QObject
{
    Q_OBJECT
public:
    explicit ScreenGrabber(QObject* parent = nullptr);
    enum class PortalStatus
    {
        Success,
        Unavailable,
        Failed
    };
    QPixmap grabEntireDesktop(bool& ok, int preSelectedMonitor = -1);
    QPixmap grabFullDesktop(bool& ok);
    QRect screenGeometry(QScreen* screen);
    QPixmap grabScreen(QScreen* screenNumber, bool& ok);
    PortalStatus freeDesktopPortal(QPixmap& res, QString& errorDetail);
    QRect desktopGeometry();
    QRect logicalDesktopGeometry();
    // Returns the monitor the user actually configured as primary. On GNOME
    // Wayland QGuiApplication::primaryScreen() is unreliable (Qt's wayland
    // plugin does not implement the wp_primary_output protocol, so it reports
    // the first wl_output instead), therefore the real primary is queried
    // from mutter. Falls back to QGuiApplication::primaryScreen() everywhere
    // else.
    static QScreen* reliablePrimaryScreen();
    // Returns the monitor containing the pointer. On X11 the global cursor
    // position is reliable. On Wayland compositors do not expose the global
    // pointer position before a surface is mapped, but they place new windows
    // on the monitor containing the pointer, so a tiny invisible probe window
    // is mapped and the screen the compositor assigned to it is returned.
    QScreen* cursorMonitor();
    int getSelectedMonitor() const { return m_selectedMonitor; }
    QScreen* getSelectedScreen() const;
    QPixmap selectMonitorAndCrop(const QPixmap& fullScreenshot, bool& ok);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    void adjustDevicePixelRatio(QPixmap& pixmap);
    QList<QWidget*> createMonitorPreviews(const QPixmap& fullScreenshot,
                                          QScreen* cursorScreen);
    void cancelMonitorSelection();
    void moveHighlightedMonitorPreview(int offset);
    void selectHighlightedMonitorPreview();
    void selectMonitor(int monitorIndex);
    // m_highlightedMonitorPreview holds a monitor index (not a preview index)
    // because every picker window shows one preview per monitor.
    void setHighlightedMonitorPreview(int monitorIndex);
    QPixmap cropToMonitor(const QPixmap& fullScreenshot, int monitorIndex);
    QPixmap windowsScreenshot(int wid);
    QPixmap x11LegacyScreenshot();
    QPixmap unixScreenshot(bool& ok);

    DesktopInfo m_info;
    QPixmap Screenshot;
    int m_selectedMonitor;
    int m_highlightedMonitorPreview;
    QList<MonitorPreview*> m_monitorPreviews;
    QEventLoop* m_monitorSelectionLoop;
    bool m_userCancelled;
    static bool m_monitorSelectionActive;
};
