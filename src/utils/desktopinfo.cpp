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

bool DesktopInfo::waylandDetected()
{
    return XDG_SESSION_TYPE == QLatin1String("wayland") ||
           WAYLAND_DISPLAY.contains(QLatin1String("wayland"),
                                    Qt::CaseInsensitive);
}

DesktopInfo::WM DesktopInfo::windowManager()
{
    DesktopInfo::WM res = DesktopInfo::OTHER;
    QStringList desktops = XDG_CURRENT_DESKTOP.split(QChar(':'));
    for (auto& desktop : desktops) {
        if (desktop.contains(QLatin1String("GNOME"), Qt::CaseInsensitive)) {
            return DesktopInfo::GNOME;
        }
        if (desktop.contains(QLatin1String("sway"), Qt::CaseInsensitive)) {
            return DesktopInfo::SWAY;
        }
        if (desktop.contains(QLatin1String("kde-plasma"))) {
            return DesktopInfo::KDE;
        }
    }

    if (!GNOME_DESKTOP_SESSION_ID.isEmpty()) {
        return DesktopInfo::GNOME;
    }

    if (!KDE_FULL_SESSION.isEmpty()) {
        return DesktopInfo::KDE;
    }

    return res;
}
