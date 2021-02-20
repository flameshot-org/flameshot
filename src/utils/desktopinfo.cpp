// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "desktopinfo.h"
#include <QProcessEnvironment>

DesktopInfo::DesktopInfo()
{
    auto e = QProcessEnvironment::systemEnvironment();
    XDG_CURRENT_DESKTOP = e.value(QStringLiteral("XDG_CURRENT_DESKTOP"));
    XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
    WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));
    KDE_FULL_SESSION = e.value(QStringLiteral("KDE_FULL_SESSION"));
    GNOME_DESKTOP_SESSION_ID =
      e.value(QStringLiteral("GNOME_DESKTOP_SESSION_ID"));
    DESKTOP_SESSION = e.value(QStringLiteral("DESKTOP_SESSION"));
}

bool DesktopInfo::waylandDectected()
{
    return XDG_SESSION_TYPE == QLatin1String("wayland") ||
           WAYLAND_DISPLAY.contains(QLatin1String("wayland"),
                                    Qt::CaseInsensitive);
}

DesktopInfo::WM DesktopInfo::windowManager()
{
    DesktopInfo::WM res = DesktopInfo::OTHER;
    if (XDG_CURRENT_DESKTOP.contains(QLatin1String("GNOME"),
                                     Qt::CaseInsensitive) ||
        !GNOME_DESKTOP_SESSION_ID.isEmpty()) {
        res = DesktopInfo::GNOME;
    } else if (XDG_CURRENT_DESKTOP.contains(QLatin1String("sway"),
                                            Qt::CaseInsensitive)) {
        res = DesktopInfo::SWAY;
    } else if (!KDE_FULL_SESSION.isEmpty() ||
               DESKTOP_SESSION == QLatin1String("kde-plasma")) {
        res = DesktopInfo::KDE;
    }
    return res;
}
