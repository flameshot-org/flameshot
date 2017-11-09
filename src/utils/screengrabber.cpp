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

#include "screengrabber.h"
#include <QPixmap>
#include <QScreen>
#include <QGuiApplication>
#include <QApplication>
#include <QDesktopWidget>

ScreenGrabber::ScreenGrabber(QObject *parent) : QObject(parent) {

}

QPixmap ScreenGrabber::grabEntireDesktop(bool &ok) {
    ok = true; // revisit later
    if(m_info.waylandDectected()) {
        ok = false;
        QPixmap res;
        /*
        habdle screenshot based on DE

        switch (m_info.windowManager()) {
        case m_info.GNOME:
            // https://github.com/GNOME/gnome-shell/blob/695bfb96160033be55cfb5ac41c121998f98c328/data/org.gnome.Shell.Screenshot.xml
            break;
        case m_info.KDE:
            // https://github.com/KDE/spectacle/blob/517a7baf46a4ca0a45f32fd3f2b1b7210b180134/src/PlatformBackends/KWinWaylandImageGrabber.cpp#L145
            break;
        default:
            ok = false;
            break;
        }
        */
        return res;
    }
    QRect geometry;
    for (QScreen *const screen : QGuiApplication::screens()) {
        geometry = geometry.united(screen->geometry());
    }

    QPixmap p(QApplication::primaryScreen()->grabWindow(
                  QApplication::desktop()->winId(),
                  geometry.x(),
                  geometry.y(),
                  geometry.width(),
                  geometry.height())
              );
    p.setDevicePixelRatio(QApplication::desktop()->devicePixelRatio());
    return p;
}
