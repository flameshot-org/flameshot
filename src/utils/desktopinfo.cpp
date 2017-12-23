// Copyright 2017 Alejandro Sirgo Rica
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

DesktopInfo::DesktopInfo()
{
    auto e = QProcessEnvironment::systemEnvironment();
    XDG_CURRENT_DESKTOP = e.value("XDG_CURRENT_DESKTOP");
    XDG_SESSION_TYPE = e.value("XDG_SESSION_TYPE");
    WAYLAND_DISPLAY = e.value("WAYLAND_DISPLAY");
    KDE_FULL_SESSION = e.value("KDE_FULL_SESSION");
    GNOME_DESKTOP_SESSION_ID = e.value("GNOME_DESKTOP_SESSION_ID");
    DESKTOP_SESSION = e.value("DESKTOP_SESSION");
}

bool DesktopInfo::waylandDectected() {
    return XDG_SESSION_TYPE == "wayland" ||
            WAYLAND_DISPLAY.contains("wayland", Qt::CaseInsensitive);
}

DesktopInfo::WM DesktopInfo::windowManager() {
    DesktopInfo::WM res = DesktopInfo::OTHER;
    if (XDG_CURRENT_DESKTOP.contains("GNOME", Qt::CaseInsensitive) ||
            !GNOME_DESKTOP_SESSION_ID.isEmpty())
    {
        res = DesktopInfo::GNOME;
    } else if (!KDE_FULL_SESSION.isEmpty() || DESKTOP_SESSION == "kde-plasma") {
        res = DesktopInfo::KDE;
    }
    return res;
}
