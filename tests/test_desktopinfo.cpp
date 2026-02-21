// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#include "src/utils/desktopinfo.h"
#include <QTest>

class TestDesktopInfo : public QObject
{
    Q_OBJECT

private:
    void clearEnv()
    {
        qputenv("XDG_CURRENT_DESKTOP", "");
        qputenv("XDG_SESSION_TYPE", "");
        qputenv("WAYLAND_DISPLAY", "");
        qputenv("KDE_FULL_SESSION", "");
        qputenv("GNOME_DESKTOP_SESSION_ID", "");
        qputenv("DESKTOP_SESSION", "");
    }

private slots:

    void init()
    {
        clearEnv();
    }

    void waylandDetected_viaSessionType()
    {
        qputenv("XDG_SESSION_TYPE", "wayland");
        DesktopInfo info;
        QVERIFY(info.waylandDetected());
    }

    void waylandDetected_viaWaylandDisplay()
    {
        qputenv("WAYLAND_DISPLAY", "wayland-0");
        DesktopInfo info;
        QVERIFY(info.waylandDetected());
    }

    void waylandDetected_x11Session()
    {
        qputenv("XDG_SESSION_TYPE", "x11");
        DesktopInfo info;
        QVERIFY(!info.waylandDetected());
    }

    void waylandDetected_emptyEnv()
    {
        DesktopInfo info;
        QVERIFY(!info.waylandDetected());
    }

    void windowManager_gnome()
    {
        qputenv("XDG_CURRENT_DESKTOP", "GNOME");
        DesktopInfo info;
        QCOMPARE(info.windowManager(), DesktopInfo::GNOME);
    }

    void windowManager_gnomeViaSessionId()
    {
        qputenv("GNOME_DESKTOP_SESSION_ID", "this-is-deprecated");
        DesktopInfo info;
        QCOMPARE(info.windowManager(), DesktopInfo::GNOME);
    }

    void windowManager_kde()
    {
        qputenv("XDG_CURRENT_DESKTOP", "kde-plasma");
        DesktopInfo info;
        QCOMPARE(info.windowManager(), DesktopInfo::KDE);
    }

    void windowManager_kdeViaFullSession()
    {
        qputenv("KDE_FULL_SESSION", "true");
        DesktopInfo info;
        QCOMPARE(info.windowManager(), DesktopInfo::KDE);
    }

    void windowManager_hyprland()
    {
        qputenv("XDG_CURRENT_DESKTOP", "Hyprland");
        DesktopInfo info;
        QCOMPARE(info.windowManager(), DesktopInfo::HYPRLAND);
    }

    void windowManager_cosmic()
    {
        qputenv("XDG_CURRENT_DESKTOP", "cosmic");
        DesktopInfo info;
        QCOMPARE(info.windowManager(), DesktopInfo::COSMIC);
    }

    void windowManager_sway()
    {
        qputenv("XDG_CURRENT_DESKTOP", "sway");
        DesktopInfo info;
        QCOMPARE(info.windowManager(), DesktopInfo::WLROOTS);
    }

    void windowManager_river()
    {
        qputenv("XDG_CURRENT_DESKTOP", "river");
        DesktopInfo info;
        QCOMPARE(info.windowManager(), DesktopInfo::WLROOTS);
    }

    void windowManager_qtile()
    {
        qputenv("XDG_CURRENT_DESKTOP", "qtile");
        DesktopInfo info;
        QCOMPARE(info.windowManager(), DesktopInfo::QTILE);
    }

    void windowManager_unknown()
    {
        qputenv("XDG_CURRENT_DESKTOP", "i3");
        DesktopInfo info;
        QCOMPARE(info.windowManager(), DesktopInfo::OTHER);
    }

    void windowManager_colonSeparated()
    {
        qputenv("XDG_CURRENT_DESKTOP", "ubuntu:GNOME");
        DesktopInfo info;
        QCOMPARE(info.windowManager(), DesktopInfo::GNOME);
    }
};

QTEST_GUILESS_MAIN(TestDesktopInfo)
#include "test_desktopinfo.moc"
