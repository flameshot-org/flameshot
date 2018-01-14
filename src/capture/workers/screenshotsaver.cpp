// Copyright(c) 2017-2018 Alejandro Sirgo Rica & Contributors
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
#include <QFileDialog>
#include <QImageWriter>

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

void ScreenshotSaver::saveToFilesystemGUI(const QPixmap &capture) {
    bool ok = false;
    while (!ok) {
        QString savePath = QFileDialog::getSaveFileName(
                    nullptr,
                    QString(),
                    FileNameHandler().absoluteSavePath() + ".png");

        if (savePath.isNull()) {
            return;
        }
        ok = capture.save(savePath);
        if (ok) {
            QString pathNoFile = savePath.left(savePath.lastIndexOf("/"));
            ConfigHandler().setSavePath(pathNoFile);
            QString msg = QObject::tr("Capture saved as ") + savePath;
            SystemNotification().sendMessage(msg);
        } else {
            QString msg = QObject::tr("Error trying to save as ") + savePath;
            QMessageBox saveErrBox(
                        QMessageBox::Warning,
                        QObject::tr("Save Error"),
                        msg);
            saveErrBox.setWindowIcon(QIcon(":img/flameshot.png"));
            saveErrBox.exec();
        }
    }
}


