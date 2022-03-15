// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "flameshotdbusadapter.h"
#include "src/core/flameshotdaemon.h"

FlameshotDBusAdapter::FlameshotDBusAdapter(QObject* parent)
  : QDBusAbstractAdaptor(parent)
{}

FlameshotDBusAdapter::~FlameshotDBusAdapter() = default;

void FlameshotDBusAdapter::attachScreenshotToClipboard(const QByteArray& data)
{
    FlameshotDaemon::instance()->attachScreenshotToClipboard(data);
}

void FlameshotDBusAdapter::attachTextToClipboard(QString text,
                                                 QString notification)
{
    FlameshotDaemon::instance()->attachTextToClipboard(text, notification);
}

void FlameshotDBusAdapter::attachPin(const QByteArray& data)
{
    FlameshotDaemon::instance()->attachPin(data);
}
