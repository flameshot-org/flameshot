// Copyright 2017 Alejandro Sirgo Rica
//
// This file is part of Flameshot.
//
//     Flameshot is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     Flameshot is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with Flameshot.  If not, see <http://www.gnu.org/licenses/>.

#include "screenshotsaver.h"
#include "src/utils/systemnotification.h"
#include "src/utils/filenamehandler.h"
#include "src/utils/confighandler.h"
#include <QClipboard>
#include <QApplication>
#include <QMessageBox>

ScreenshotSaver::ScreenshotSaver()
{
}

void ScreenshotSaver::saveToClipboard(const QPixmap &capture) {
    QApplication::clipboard()->setPixmap(capture);
}

void ScreenshotSaver::saveToFilesystem(const QPixmap &capture,
                                       const QString &path)
{
    QString completePath = FileNameHandler().generateAbsolutePath(path);
    completePath += ".png";
    bool ok = capture.save(completePath);
    QString saveMessage;
    if (ok) {
        ConfigHandler().setSavePath(path);
        saveMessage = QObject::tr("Capture saved as ") + completePath;
    } else {
        saveMessage = QObject::tr("Error trying to save as ") + completePath;
    }
    SystemNotification().sendMessage(saveMessage);
}


