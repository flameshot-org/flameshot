// Copyright(c) 2017-2019 Alejandro Sirgo Rica & Contributors
//
// This file is part of Flameshot.
//
//     Flameshot is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     Flameshot is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with Flameshot.  If not, see <http://www.gnu.org/licenses/>.

#include "desktopinfo.h"
#include <QProcessEnvironment>

DesktopInfo::DesktopInfo() {
    auto e = QProcessEnvironment::systemEnvironment();
    XDG_CURRENT_DESKTOP = e.value(QStringLiteral("XDG_CURRENT_DESKTOP"));
    XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
    WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));
    KDE_FULL_SESSION = e.value(QStringLiteral("KDE_FULL_SESSION"));
    GNOME_DESKTOP_SESSION_ID = e.value(QStringLiteral("GNOME_DESKTOP_SESSION_ID"));
    DESKTOP_SESSION = e.value(QStringLiteral("DESKTOP_SESSION"));
}

bool DesktopInfo::waylandDectected() {
    return XDG_SESSION_TYPE == QLatin1String("wayland") ||
            WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive);
}

DesktopInfo::WM DesktopInfo::windowManager() {
    DesktopInfo::WM res = DesktopInfo::OTHER;
    if (XDG_CURRENT_DESKTOP.contains(QLatin1String("GNOME"), Qt::CaseInsensitive) ||
            !GNOME_DESKTOP_SESSION_ID.isEmpty())
    {
        res = DesktopInfo::GNOME;
    } else if (!KDE_FULL_SESSION.isEmpty() || DESKTOP_SESSION == QLatin1String("kde-plasma")) {
        res = DesktopInfo::KDE;
    }
    return res;
}
