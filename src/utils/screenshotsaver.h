// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QColorSpace>
#include <QString>
#include <QWidget>

class QPixmap;

// The optional colorSpace embeds an ICC profile in the written image when valid
// (see ColorProfileProvider); an invalid color space keeps the previous,
// untagged behavior.
bool saveToFilesystem(const QPixmap& capture,
                      const QString& path,
                      const QString& messagePrefix = "",
                      const QColorSpace& colorSpace = {});
QString ShowSaveFileDialog(const QString& title, const QString& directory);
void saveToClipboardMime(const QPixmap& capture,
                         const QString& imageType,
                         const QColorSpace& colorSpace = {});
void saveToClipboard(const QPixmap& capture, const QColorSpace& colorSpace = {});
// GNOME Wayland: keeps the widget alive until clipboard data is fetched
bool saveToClipboardGnomeWorkaround(const QPixmap& pixmap, QWidget* keepAlive);
bool saveToFilesystemGUI(const QPixmap& capture,
                         const QColorSpace& colorSpace = {});
