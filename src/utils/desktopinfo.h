#ifndef DESKTOPINFO_H
#define DESKTOPINFO_H

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

#endif // DESKTOPINFO_H
