// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#include "qrplugin.h"
#include "qrtool.h"

QrPlugin::QrPlugin(QObject* parent)
  : QObject(parent)
{}

QString QrPlugin::pluginId() const
{
    return QStringLiteral("org.flameshot.plugin.qr");
}

QString QrPlugin::pluginName() const
{
    return tr("QR & Barcode Scanner");
}

QString QrPlugin::pluginVersion() const
{
    return QStringLiteral("1.0.0");
}

QString QrPlugin::pluginAuthor() const
{
    return QStringLiteral("Sniper Ravan");
}

QString QrPlugin::pluginDescription() const
{
    return tr("Fast QR code and barcode scanner with 1-click URL navigation "
              "and clipboard copying.");
}

QIcon QrPlugin::pluginIcon() const
{
    return QIcon(QStringLiteral(":/qrplugin/icons/white_qr.svg"));
}

QList<CaptureTool*> QrPlugin::createTools(QObject* parent)
{
    return { new QrTool(parent) };
}
