// Copyright(c) 2017-2018 Alejandro Sirgo Rica & Contributors
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

#pragma once

#include <QString>

class DesktopInfo
{
public:
    DesktopInfo();

    enum WM {
        GNOME,
        KDE,
        OTHER
    };

    bool waylandDectected();
    WM windowManager();

private:
    QString XDG_CURRENT_DESKTOP;
    QString XDG_SESSION_TYPE;
    QString WAYLAND_DISPLAY;
    QString KDE_FULL_SESSION;
    QString GNOME_DESKTOP_SESSION_ID;
    QString GDMSESSION;
    QString DESKTOP_SESSION;

};
