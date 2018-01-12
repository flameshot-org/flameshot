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

#include "flameshotdbusadapter.h"
#include "src/utils/confighandler.h"
#include "src/utils/screengrabber.h"
#include "src/core/controller.h"
#include "src/core/resourceexporter.h"
#include "src/utils/systemnotification.h"
#include <QTimer>
#include <functional>
#include <QBuffer>

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
    auto controller =  Controller::getInstance();
    connect(controller, &Controller::captureFailed,
            this, &FlameshotDBusAdapter::captureFailed);
    connect(controller, &Controller::captureTaken,
            this, &FlameshotDBusAdapter::captureTaken);
}

FlameshotDBusAdapter::~FlameshotDBusAdapter() {

}

void FlameshotDBusAdapter::graphicCapture(QString path, int delay, uint id) {
    auto controller =  Controller::getInstance();

    auto f = [controller, id, path, this]() {
       controller->createVisualCapture(id, path);
    };
    // QTimer::singleShot(delay, controller, f); // requires Qt 5.4
    doLater(delay, controller, f);
}

void FlameshotDBusAdapter::fullScreen(
        QString path, bool toClipboard, int delay, uint id)
{
    auto f = [id, path, toClipboard, this]() {
        bool ok = true;
        QPixmap p(ScreenGrabber().grabEntireDesktop(ok));
        if (!ok) {
            SystemNotification().sendMessage(tr("Unable to capture screen"));
            Q_EMIT captureFailed(id);
            return;
        }
        if(toClipboard) {
            ResourceExporter().captureToClipboard(p);
        }
        if(!path.isEmpty()) {
            ResourceExporter().captureToFile(p, path);
        }
        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        p.save(&buffer, "PNG");
        Q_EMIT captureTaken(id, byteArray);
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
