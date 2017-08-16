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
#include "src/core/controller.h"
#include <QTimer>

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
    QTimer::singleShot(delay, controller, f);
}

void FlameshotDBusAdapter::fullScreen(QString path, bool toClipboard, int delay) {
    auto controller =  Controller::getInstance();
    auto f = [controller, path, toClipboard, this]() {
        controller->saveFullScreenshot(path, toClipboard);
    };
    QTimer::singleShot(delay, controller, f);

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
