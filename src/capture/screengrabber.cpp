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

QPixmap ScreenGrabber::grabEntireDesktop() {
    QRect geometry;
    for (QScreen *screen : QGuiApplication::screens()) {
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
