// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QString>

class QPixmap;
class QWidget;

class ScreenshotSaver
{
public:
    ScreenshotSaver();
    ScreenshotSaver(const unsigned id);

    void saveToClipboard(const QPixmap& capture);
    void saveToClipboardMime(const QPixmap& capture, const QString& imageType);
    bool saveToFilesystem(const QPixmap& capture,
                          const QString& path,
                          const QString& messagePrefix);
    bool saveToFilesystemGUI(const QPixmap& capture);

private:
    unsigned m_id;
    QString ShowSaveFileDialog(QWidget* parent,
                               const QString& title,
                               const QString& directory,
                               const QString& filter);

    const QString pngFilter = "Portable Network Graphic file (PNG) (*.png)";
    const QString bmpFilter = "BMP file (*.bmp)";
    const QString jpgFilter = "JPEG file (*.jpg)";
    const QString defaultFilter = "By extension [default: *.png] (*.png)";
    const QString separator = ";;";
};
