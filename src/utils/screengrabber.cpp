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

#include "screengrabber.h"
#include "src/utils/filenamehandler.h"
#include "src/utils/systemnotification.h"
#include <QPixmap>
#include <QScreen>
#include <QGuiApplication>
#include <QApplication>
#include <QDesktopWidget>

#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
#include <QDBusInterface>
#include <QDBusReply>
#include <QDir>
#endif

ScreenGrabber::ScreenGrabber(QObject *parent) : QObject(parent) {

}

QPixmap ScreenGrabber::grabEntireDesktop(bool &ok) {
    ok = true;
#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
    if(m_info.waylandDectected()) {
        QPixmap res;
        // handle screenshot based on DE
        switch (m_info.windowManager()) {
        case DesktopInfo::GNOME: {
            // https://github.com/GNOME/gnome-shell/blob/695bfb96160033be55cfb5ac41c121998f98c328/data/org.gnome.Shell.Screenshot.xml
            QString path = FileNameHandler().generateAbsolutePath(QDir::tempPath()) + ".png";
            QDBusInterface gnomeInterface(QStringLiteral("org.gnome.Shell"),
                                          QStringLiteral("/org/gnome/Shell/Screenshot"),
                                          QStringLiteral("org.gnome.Shell.Screenshot"));
            QDBusReply<bool> reply = gnomeInterface.call(QStringLiteral("Screenshot"), false, false, path);
            if (reply.value()) {
                res = QPixmap(path);
                QFile dbusResult(path);
                dbusResult.remove();
            } else {
                ok = false;
            }
            break;
        } case DesktopInfo::KDE: {
            // https://github.com/KDE/spectacle/blob/517a7baf46a4ca0a45f32fd3f2b1b7210b180134/src/PlatformBackends/KWinWaylandImageGrabber.cpp#L145
            QDBusInterface kwinInterface(QStringLiteral("org.kde.KWin"),
                                         QStringLiteral("/Screenshot"),
                                         QStringLiteral("org.kde.kwin.Screenshot"));
            QDBusReply<QString> reply = kwinInterface.call(QStringLiteral("screenshotFullscreen"));
            res = QPixmap(reply.value());
            if (!res.isNull()) {
                QFile dbusResult(reply.value());
                dbusResult.remove();
            }
            break;
        } default:
            ok = false;
            break;
        }
        if (!ok) {
            SystemNotification().sendMessage(tr("Unable to capture screen"));
        }
        return res;
    }
#endif

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
    auto screenNumber = QApplication::desktop()->screenNumber();
    QScreen *screen = QApplication::screens()[screenNumber];
    p.setDevicePixelRatio(screen->devicePixelRatio());
    return p;
}

QPixmap ScreenGrabber::grabScreen(int screenNumber, bool &ok) {
    QPixmap p;
    bool isVirtual = QApplication::desktop()->isVirtualDesktop();
    if (isVirtual || m_info.waylandDectected()) {
        p = grabEntireDesktop(ok);
        if (ok) {
            QPoint topLeft(0, 0);
#ifdef Q_OS_WIN
            for (QScreen *const screen : QGuiApplication::screens()) {
                QPoint topLeftScreen = screen->geometry().topLeft();
                if (topLeft.x() > topLeftScreen.x() ||
                        topLeft.y() > topLeftScreen.y()) {
                    topLeft = topLeftScreen;
                }
            }
#endif
            QRect geometry = QApplication::desktop()->
                    screenGeometry(screenNumber);
            geometry.moveTo(geometry.topLeft() - topLeft);
            p = p.copy(geometry);
        }
    } else {
        p = QApplication::desktop()->screen(screenNumber)->grab();
        ok = true;
    }
    return p;
}
