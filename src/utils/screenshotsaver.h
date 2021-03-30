// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

class QPixmap;
class QString;
class QWidget;

class ScreenshotSaver
{
public:
    ScreenshotSaver();
    ScreenshotSaver(const unsigned id);

    void saveToClipboard(const QPixmap& capture);
    bool saveToFilesystem(const QPixmap& capture,
                          const QString& path,
                          const QString& messagePrefix);
    bool saveToFilesystemGUI(const QPixmap& capture);

private:
    unsigned m_id;
    QString ShowSaveFileDialog(QWidget *parent,
    		const QString &title,
    		const QString &directory,
    		const QString &filter);
};
