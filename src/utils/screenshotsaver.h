// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QString>

class QPixmap;

bool saveToFilesystem(const QPixmap& capture,
                      const QString& path,
                      const QString& messagePrefix = "");
QString ShowSaveFileDialog(const QString& title, const QString& directory);
void saveToClipboardMime(const QPixmap& capture, const QString& imageType);
void saveToClipboard(const QPixmap& capture);
bool saveToFilesystemGUI(const QPixmap& capture);
