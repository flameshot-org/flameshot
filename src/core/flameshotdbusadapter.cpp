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

#include "flameshotdbusadapter.h"
#include "src/utils/confighandler.h"
#include "src/utils/screengrabber.h"
#include "src/core/controller.h"
#include "src/core/resourceexporter.h"
#include <QTimer>
#include <functional>

namespace {
    using std::function;
    using lambda = function<void(void)>;

    // replace QTimer::singleShot introduced in QT 5.4
    // the actual target QT version is QT 5.3
    void doLater(int msec, QObject *receiver, lambda func) {
        QTimer *timer = new QTimer(receiver);
        QObject::connect(timer, &QTimer::timeout, receiver,
                         [timer, func](){ func(); timer->deleteLater(); });
        timer->setInterval(msec);
        timer->start();
    }
}

FlameshotDBusAdapter::FlameshotDBusAdapter(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{

}

FlameshotDBusAdapter::~FlameshotDBusAdapter() {

}

void FlameshotDBusAdapter::graphicCapture(QString path, int delay) {
    auto controller =  Controller::getInstance();
    auto f = [controller, path, this]() {
       controller->createVisualCapture(path);
    };
    // QTimer::singleShot(delay, controller, f); // requires Qt 5.4
    doLater(delay, controller, f);
}

void FlameshotDBusAdapter::fullScreen(QString path, bool toClipboard, int delay) {
    auto f = [path, toClipboard, this]() {
        QPixmap p(ScreenGrabber().grabEntireDesktop());
        if(toClipboard) {
            ResourceExporter().captureToClipboard(p);
        }
        if(path.isEmpty()) {
            ResourceExporter().captureToFileUi(p);
        } else {
            ResourceExporter().captureToFile(p, path);
        }
    };
    //QTimer::singleShot(delay, this, f); // // requires Qt 5.4
    doLater(delay, this, f);
}

void FlameshotDBusAdapter::openConfig() {
    Controller::getInstance()->openConfigWindow();
}

void FlameshotDBusAdapter::trayIconEnabled(bool enabled) {
    auto controller =  Controller::getInstance();
    if (enabled) {
        controller->enableTrayIcon();
    } else {
        controller->disableTrayIcon();
    }
}
