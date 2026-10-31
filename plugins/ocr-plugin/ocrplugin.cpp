// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Flameshot Contributors

#include "ocrplugin.h"
#include "ocrtool.h"

OcrPlugin::OcrPlugin(QObject* parent)
  : QObject(parent)
{}

QString OcrPlugin::pluginId() const
{
    return QStringLiteral("org.flameshot.plugin.ocr");
}

QString OcrPlugin::pluginName() const
{
    return QStringLiteral("Text Recognition (OCR)");
}

QString OcrPlugin::pluginVersion() const
{
    return QStringLiteral("1.0.0");
}

QString OcrPlugin::pluginAuthor() const
{
    return QStringLiteral("Sniper Ravan");
}

QString OcrPlugin::pluginDescription() const
{
    return QStringLiteral(
      "Extracts text and emojis from captured screenshots using Tesseract OCR "
      "with multi-language and interactive canvas support.");
}

QIcon OcrPlugin::pluginIcon() const
{
    return QIcon(QStringLiteral(":/ocrplugin/icons/white_ocr.svg"));
}

QList<CaptureTool*> OcrPlugin::createTools(QObject* parent)
{
    return { new OcrTool(parent) };
}
