// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "screenshotsaver.h"
#include "src/core/controller.h"
#include "src/utils/confighandler.h"
#include "src/utils/filenamehandler.h"
#include "src/utils/systemnotification.h"
#include "utils/desktopinfo.h"
#include <QApplication>
#include <QBuffer>
#include <QClipboard>
#include <QFileDialog>
#include <QImageWriter>
#include <QMessageBox>
#include <QMimeData>
#include <QStandardPaths>
#include <qimagewriter.h>
#include <qmimedatabase.h>
#if defined(Q_OS_MACOS)
#include "src/widgets/capture/capturewidget.h"
#endif

ScreenshotSaver::ScreenshotSaver()
  : m_id(0)
{}

ScreenshotSaver::ScreenshotSaver(const unsigned id)
  : m_id(id)
{}

void ScreenshotSaver::saveToClipboardMime(const QPixmap& capture,
                                          const QString& imageType)
{
    QByteArray array;
    QBuffer buffer{ &array };
    QImageWriter imageWriter{ &buffer, imageType.toUpper().toUtf8() };
    imageWriter.write(capture.toImage());

    QPixmap pngPixmap;
    bool isLoaded =
      pngPixmap.loadFromData(reinterpret_cast<uchar*>(array.data()),
                             array.size(),
                             imageType.toUpper().toUtf8());
    if (isLoaded) {
        QMimeData* mimeData = new QMimeData;
        mimeData->setData("image/" + imageType, array);
        QApplication::clipboard()->setMimeData(mimeData);
    } else {
        SystemNotification().sendMessage(
          QObject::tr("Error while saving to clipboard"));
    }
}

// TODO: If data is saved to the clipboard before the notification is sent via
// dbus, the application freezes.
void ScreenshotSaver::saveToClipboard(const QPixmap& capture)
{
    // If we are able to properly save the file, save the file and copy to
    // clipboard.
    if ((ConfigHandler().saveAfterCopy()) &&
        (!ConfigHandler().savePath().isEmpty())) {
        saveToFilesystem(capture,
                         ConfigHandler().savePath(),
                         QObject::tr("Capture saved to clipboard."));
    } else {
        SystemNotification().sendMessage(
          QObject::tr("Capture saved to clipboard."));
    }
    if (ConfigHandler().useJpgForClipboard()) {
        // FIXME - it doesn't work on MacOS
        saveToClipboardMime(capture, "jpeg");
    } else {
        // Need to send message before copying to clipboard
#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
        if (DesktopInfo().waylandDetected()) {
            saveToClipboardMime(capture, "png");
        } else {
            QApplication::clipboard()->setPixmap(capture);
        }
#else
        QApplication::clipboard()->setPixmap(capture);
#endif
    }
}

bool ScreenshotSaver::saveToFilesystem(const QPixmap& capture,
                                       const QString& path,
                                       const QString& messagePrefix)
{
    QString completePath = FileNameHandler().properScreenshotPath(
      path, ConfigHandler().setSaveAsFileExtension());
    QFile file{ completePath };
    file.open(QIODevice::WriteOnly);
    bool ok = capture.save(&file);
    QString saveMessage = messagePrefix;
    QString notificationPath = completePath;
    if (!saveMessage.isEmpty()) {
        saveMessage += " ";
    }

    if (ok) {
        saveMessage += QObject::tr("Capture saved as ") + completePath;
        Controller::getInstance()->sendCaptureSaved(
          m_id, QFileInfo(completePath).canonicalFilePath());
    } else {
        saveMessage += QObject::tr("Error trying to save as ") + completePath;
        if (file.error() != QFile::NoError) {
            saveMessage += ": " + file.errorString();
        }
        notificationPath = "";
    }

    SystemNotification().sendMessage(saveMessage, notificationPath);
    return ok;
}

QString ScreenshotSaver::ShowSaveFileDialog(QWidget* parent,
                                            const QString& title,
                                            const QString& directory)
{
    QFileDialog dialog(parent, title, directory);
    if (parent) {
        dialog.setWindowModality(Qt::WindowModal);
    }

    dialog.setAcceptMode(QFileDialog::AcceptSave);

    // Build string list of supported image formats
    QStringList mimeTypeList;
    foreach (auto mimeType, QImageWriter::supportedMimeTypes())
        mimeTypeList.append(mimeType);
    dialog.setMimeTypeFilters(mimeTypeList);

    QString suffix = ConfigHandler().setSaveAsFileExtension();
    QString defaultMimeType =
      QMimeDatabase().mimeTypeForFile("image." + suffix).name();
    dialog.selectMimeTypeFilter(defaultMimeType);

    if (dialog.exec() == QDialog::Accepted) {
        return dialog.selectedFiles().first();
    } else {
        return QString();
    }
}

bool ScreenshotSaver::saveToFilesystemGUI(const QPixmap& capture)
{
    bool ok = false;
    ConfigHandler config;
    QString defaultSavePath = ConfigHandler().savePath();
    if (defaultSavePath.isEmpty() || !QDir(defaultSavePath).exists() ||
        !QFileInfo(defaultSavePath).isWritable()) {
        defaultSavePath =
          QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    }
    QString savePath = FileNameHandler().properScreenshotPath(
      defaultSavePath, ConfigHandler().setSaveAsFileExtension());
#if defined(Q_OS_MACOS)
    for (QWidget* widget : qApp->topLevelWidgets()) {
        QString className(widget->metaObject()->className());
        if (0 ==
            className.compare(CaptureWidget::staticMetaObject.className())) {
            widget->showNormal();
            widget->hide();
            break;
        }
    }
#endif
    if (!config.savePathFixed()) {
        // auto imageFormats = QImageWriter::supportedImageFormats();
        savePath =
          ShowSaveFileDialog(nullptr, QObject::tr("Save screenshot"), savePath);
    }
    if (savePath == "") {
        return ok;
    }

    QFile file{ savePath };
    file.open(QIODevice::WriteOnly);

    ok = capture.save(&file);

    if (ok) {
        QString pathNoFile =
          savePath.left(savePath.lastIndexOf(QLatin1String("/")));

        ConfigHandler().setSavePath(pathNoFile);

        QString msg = QObject::tr("Capture saved as ") + savePath;

        if (config.copyPathAfterSave()) {
            msg =
              QObject::tr("Capture is saved and copied to the clipboard as ") +
              savePath;
        }

        SystemNotification().sendMessage(msg, savePath);

        Controller::getInstance()->sendCaptureSaved(
          m_id, QFileInfo(savePath).canonicalFilePath());

        if (config.copyPathAfterSave()) {
            QApplication::clipboard()->setText(savePath);
        }

    } else {
        QString msg = QObject::tr("Error trying to save as ") + savePath;

        if (file.error() != QFile::NoError) {
            msg += ": " + file.errorString();
        }

        QMessageBox saveErrBox(
          QMessageBox::Warning, QObject::tr("Save Error"), msg);
        saveErrBox.setWindowIcon(QIcon(":img/app/flameshot.svg"));
        saveErrBox.exec();
    }

    return ok;
}
