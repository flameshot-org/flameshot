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

void
ScreenshotSaver::saveToClipboard(const QPixmap& capture)
{
  if (ConfigHandler().saveAfterCopyValue()) {
    if (!ConfigHandler().saveAfterCopyPathValue().isEmpty()) {
      saveToFilesystem(capture, ConfigHandler().saveAfterCopyPathValue());
    }
  } else {
    SystemNotification().sendMessage(QObject::tr("Capture saved to clipboard"));
  }

  QApplication::clipboard()->setPixmap(capture);
}

bool
ScreenshotSaver::saveToFilesystem(const QPixmap& capture, const QString& path)
{
  QString completePath = FileNameHandler().generateAbsolutePath(path);
  completePath += QLatin1String(".png");
  bool ok = capture.save(completePath);
  QString saveMessage;
  QString notificationPath = completePath;

  if (ok) {
    ConfigHandler().setSavePath(path);
    saveMessage = QObject::tr("Capture saved as ") + completePath;
  } else {
    saveMessage = QObject::tr("Error trying to save as ") + completePath;
    notificationPath = "";
  }

  SystemNotification().sendMessage(saveMessage, notificationPath);
  return ok;
}

bool
ScreenshotSaver::saveToFilesystemGUI(const QPixmap& capture)
{
  bool ok = false;

  while (!ok) {
    QString savePath = QFileDialog::getSaveFileName(
      nullptr,
      QString(),
      FileNameHandler().absoluteSavePath() + ".png",
      QLatin1String("Portable Network Graphic file (PNG) (*.png);;BMP file "
                    "(*.bmp);;JPEG file (*.jpg)"));

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
