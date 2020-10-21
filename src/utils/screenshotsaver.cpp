// Copyright(c) 2017-2019 Alejandro Sirgo Rica & Contributors
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
#include "src/utils/confighandler.h"
#include "src/utils/filenamehandler.h"
#include "src/utils/systemnotification.h"
#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QImageWriter>
#include <QMessageBox>

ScreenshotSaver::ScreenshotSaver() {}

// TODO: If data is saved to the clipboard before the notification is sent via
// dbus, the application freezes.
void ScreenshotSaver::saveToClipboard(const QPixmap& capture)
{

    // If we are able to properly save the file, save the file and copy to
    // clipboard.
    if ((ConfigHandler().saveAfterCopyValue()) &&
        (!ConfigHandler().saveAfterCopyPathValue().isEmpty())) {
        saveToFilesystem(capture,
                         ConfigHandler().saveAfterCopyPathValue(),
                         QObject::tr("Capture saved to clipboard."));
        QApplication::clipboard()->setPixmap(capture);
    }
    // Otherwise only save to clipboard
    else {
        SystemNotification().sendMessage(
          QObject::tr("Capture saved to clipboard"));
        QApplication::clipboard()->setPixmap(capture);
    }
}

bool ScreenshotSaver::saveToFilesystem(const QPixmap& capture,
                                       const QString& path,
                                       const QString& messagePrefix)
{
    QString completePath = FileNameHandler().generateAbsolutePath(path);
    completePath += QLatin1String(".png");
    bool ok = capture.save(completePath);
    QString saveMessage;
    QString notificationPath = completePath;

    if (ok) {
        ConfigHandler().setSavePath(path);
        saveMessage =
          messagePrefix + QObject::tr("Capture saved as ") + completePath;
    } else {
        saveMessage = messagePrefix + QObject::tr("Error trying to save as ") +
                      completePath;
        notificationPath = "";
    }

    SystemNotification().sendMessage(saveMessage, notificationPath);
    return ok;
}

bool ScreenshotSaver::saveToFilesystemGUI(const QPixmap& capture)
{
    bool ok = false;
    while (!ok) {
        ConfigHandler config;
        QString savePath = FileNameHandler().absoluteSavePath();
        if (!config.savePathFixed()) {
            savePath = QFileDialog::getSaveFileName(
              nullptr,
              QObject::tr("Save screenshot"),
              FileNameHandler().absoluteSavePath(),
              QLatin1String("Portable Network Graphic file (PNG) (*.png);;BMP "
                            "file (*.bmp);;JPEG file (*.jpg)"));
        }

        if (savePath.isNull()) {
            break;
        }

        if (!savePath.endsWith(QLatin1String(".png"), Qt::CaseInsensitive) &&
            !savePath.endsWith(QLatin1String(".bmp"), Qt::CaseInsensitive) &&
            !savePath.endsWith(QLatin1String(".jpg"), Qt::CaseInsensitive)) {
            savePath += QLatin1String(".png");
        }

        ok = capture.save(savePath);

        if (ok) {
            QString pathNoFile =
              savePath.left(savePath.lastIndexOf(QLatin1String("/")));
            ConfigHandler().setSavePath(pathNoFile);
            QString msg = QObject::tr("Capture saved as ") + savePath;
            if (config.copyPathAfterSaveEnabled()) {
                QApplication::clipboard()->setText(savePath);
                msg = QObject::tr(
                        "Capture is saved and copied to the clipboard as ") +
                      savePath;
            }
            SystemNotification().sendMessage(msg, savePath);
        } else {
            QString msg = QObject::tr("Error trying to save as ") + savePath;
            QMessageBox saveErrBox(
              QMessageBox::Warning, QObject::tr("Save Error"), msg);
            saveErrBox.setWindowIcon(QIcon(":img/app/flameshot.svg"));
            saveErrBox.exec();
        }
    }
    return ok;
}
