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

FlameshotDBusAdapter::FlameshotDBusAdapter(Controller *parent)
    : QDBusAbstractAdaptor(parent)
{

}

FlameshotDBusAdapter::~FlameshotDBusAdapter() {

}

Controller *FlameshotDBusAdapter::parent() const {
    return static_cast<Controller *>(QObject::parent());
}

void FlameshotDBusAdapter::openCapture() {
    parent()->createVisualCapture();
}

void FlameshotDBusAdapter::openCaptureWithPath(QString path) {
    ConfigHandler().setSavePath(path);
    parent()->createVisualCapture(false);
}

void FlameshotDBusAdapter::fullScreen(bool toClipboard) {
    QString path = parent()->saveScreenshot(toClipboard);
    if (!path.isEmpty()) {
        QString saveMessage(tr("Capture saved in "));
        parent()->showDesktopNotification(saveMessage + path);
    }
}

void FlameshotDBusAdapter::fullScreenWithPath(QString path, bool toClipboard) {
    QString finalPath = parent()->saveScreenshot(path, toClipboard);
    QString saveMessage(tr("Capture saved in "));
    parent()->showDesktopNotification(saveMessage + finalPath);
}
