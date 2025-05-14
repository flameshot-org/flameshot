// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QString>

class DesktopInfo
{
public:
    DesktopInfo();

    enum WM
    {
        GNOME,
        KDE,
        COSMIC,
        OTHER,
        QTILE,
        WLROOTS,
        HYPRLAND
    };

    bool waylandDetected();
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
