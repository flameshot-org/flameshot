// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "flameshotdbusadapter.h"
#include "src/core/controller.h"
#include "src/core/flameshotdaemon.h"
#include "src/utils/confighandler.h"

FlameshotDBusAdapter::FlameshotDBusAdapter(QObject* parent)
  : QDBusAbstractAdaptor(parent)
{}

FlameshotDBusAdapter::~FlameshotDBusAdapter() {}

void FlameshotDBusAdapter::requestCapture(const QByteArray& requestData)
{
    CaptureRequest req = CaptureRequest::deserialize(requestData);
    Controller::getInstance()->requestCapture(req);
}

void FlameshotDBusAdapter::attachScreenshotToClipboard(const QByteArray& data)
{
    FlameshotDaemon::instance()->attachScreenshotToClipboard(data);
}

void FlameshotDBusAdapter::attachTextToClipboard(QString text)
{
    FlameshotDaemon::instance()->attachTextToClipboard(text);
}

void FlameshotDBusAdapter::attachPin(const QByteArray& data)
{
    FlameshotDaemon::instance()->attachPin(data);
}
