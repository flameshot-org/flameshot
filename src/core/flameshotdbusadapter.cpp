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
#include <QTimer>

FlameshotDBusAdapter::FlameshotDBusAdapter(Controller *parent)
    : QDBusAbstractAdaptor(parent)
{

}

FlameshotDBusAdapter::~FlameshotDBusAdapter() {

}

Controller *FlameshotDBusAdapter::parent() const {
    return static_cast<Controller *>(QObject::parent());
}

void FlameshotDBusAdapter::graphicCapture(QString path, int delay) {
    auto p = parent();
    auto f = [p, path, this]() {
        p->createVisualCapture(path);
    };
    QTimer::singleShot(delay, p, f);
}

void FlameshotDBusAdapter::fullScreen(QString path, bool toClipboard, int delay) {
    auto p = parent();
    auto f = [p, path, toClipboard, this]() {
        p->saveScreenshot(path, toClipboard);
    };
    QTimer::singleShot(delay, p, f);

}
