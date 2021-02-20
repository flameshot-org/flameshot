// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "flameshotdbusadapter.h"
#include "src/core/controller.h"
#include "src/utils/confighandler.h"
#include "src/utils/screengrabber.h"
#include "src/utils/screenshotsaver.h"
#include "src/utils/systemnotification.h"
#include <QBuffer>
FlameshotDBusAdapter::FlameshotDBusAdapter(QObject* parent)
  : QDBusAbstractAdaptor(parent)
{
    auto controller = Controller::getInstance();
    connect(controller,
            &Controller::captureFailed,
            this,
            &FlameshotDBusAdapter::captureFailed);
    connect(controller,
            &Controller::captureTaken,
            this,
            &FlameshotDBusAdapter::handleCaptureTaken);
    connect(controller,
            &Controller::captureSaved,
            this,
            &FlameshotDBusAdapter::captureSaved);
}

FlameshotDBusAdapter::~FlameshotDBusAdapter() {}

void FlameshotDBusAdapter::graphicCapture(QString path, int delay, uint id)
{
    CaptureRequest req(CaptureRequest::GRAPHICAL_MODE, delay, path);
    //    if (toClipboard) {
    //        req.addTask(CaptureRequest::CLIPBOARD_SAVE_TASK);
    //    }
    req.setStaticID(id);
    Controller::getInstance()->requestCapture(req);
}

void FlameshotDBusAdapter::fullScreen(QString path,
                                      bool toClipboard,
                                      int delay,
                                      uint id)
{
    CaptureRequest req(CaptureRequest::FULLSCREEN_MODE, delay, path);
    if (toClipboard) {
        req.addTask(CaptureRequest::CLIPBOARD_SAVE_TASK);
    }
    if (!path.isEmpty()) {
        req.addTask(CaptureRequest::FILESYSTEM_SAVE_TASK);
    }
    req.setStaticID(id);
    Controller::getInstance()->requestCapture(req);
}

void FlameshotDBusAdapter::openLauncher()
{
    Controller::getInstance()->openLauncherWindow();
}

void FlameshotDBusAdapter::captureScreen(int number,
                                         QString path,
                                         bool toClipboard,
                                         int delay,
                                         uint id)
{
    CaptureRequest req(CaptureRequest::SCREEN_MODE, delay, path, number);
    if (toClipboard) {
        req.addTask(CaptureRequest::CLIPBOARD_SAVE_TASK);
    }
    if (!path.isEmpty()) {
        req.addTask(CaptureRequest::FILESYSTEM_SAVE_TASK);
    }
    req.setStaticID(id);
    Controller::getInstance()->requestCapture(req);
}

void FlameshotDBusAdapter::openConfig()
{
    Controller::getInstance()->openConfigWindow();
}

void FlameshotDBusAdapter::trayIconEnabled(bool enabled)
{
    auto controller = Controller::getInstance();
    if (enabled) {
        controller->enableTrayIcon();
    } else {
        controller->disableTrayIcon();
    }
}

void FlameshotDBusAdapter::autostartEnabled(bool enabled)
{
    ConfigHandler().setStartupLaunch(enabled);
    auto controller = Controller::getInstance();
    // Autostart is not saved in a .ini file, requires manual update
    controller->updateConfigComponents();
}

void FlameshotDBusAdapter::handleCaptureTaken(uint id,
                                              const QPixmap& p,
                                              QRect selection)
{
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    p.save(&buffer, "PNG");
    emit captureTaken(id, byteArray, selection);
}
