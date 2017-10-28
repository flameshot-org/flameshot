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
    GDMSESSION = e.value("GDMSESSION");
    DESKTOP_SESSION = e.value("DESKTOP_SESSION");
}

bool DesktopInfo::waylandDectected() {
    return XDG_SESSION_TYPE == "wayland" || WAYLAND_DISPLAY.contains("wayland");
}

DesktopInfo::WM DesktopInfo::windowManager() {
    DesktopInfo::WM res = DesktopInfo::OTHER;
    if (XDG_CURRENT_DESKTOP == "GNOME" ||
            (!GNOME_DESKTOP_SESSION_ID.isEmpty() &&
             "this-is-deprecated" != GNOME_DESKTOP_SESSION_ID) ||
            QString::compare("GNOME", GDMSESSION, Qt::CaseInsensitive) == 0 ||
            GDMSESSION == "gnome-shell" ||
            GDMSESSION == "gnome-classic" ||
            GDMSESSION == "gnome-fallback")
    {
        res = DesktopInfo::GNOME;
    } else if (KDE_FULL_SESSION == "true" || DESKTOP_SESSION == "kde-plasma") {
        res = DesktopInfo::KDE;
    }
    return res;
}
