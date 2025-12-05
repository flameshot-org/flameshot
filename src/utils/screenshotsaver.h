// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QString>
#include <QWidget>

class QPixmap;

bool saveToFilesystem(const QPixmap& capture,
                      const QString& path,
                      const QString& messagePrefix = "");
QString ShowSaveFileDialog(const QString& title, const QString& directory);
void saveToClipboardMime(const QPixmap& capture, const QString& imageType);
void saveToClipboard(const QPixmap& capture);
// GNOME Wayland: keeps the widget alive until clipboard data is fetched
bool saveToClipboardGnomeWorkaround(const QPixmap& pixmap, QWidget* keepAlive);
bool saveToFilesystemGUI(const QPixmap& capture);
