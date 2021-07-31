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
    if ((ConfigHandler().saveAfterCopyValue()) &&
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
        if (DesktopInfo().waylandDectected()) {
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
    QString completePath = FileNameHandler().generateAbsolutePath(path);
    completePath += QLatin1String(".png");
    bool ok = capture.save(completePath);
    QString saveMessage = messagePrefix;
    QString notificationPath = completePath;
    if (!saveMessage.isEmpty()) {
        saveMessage += " ";
    }

    if (ok) {
        ConfigHandler().setSavePath(path);
        saveMessage += QObject::tr("Capture saved as ") + completePath;
        Controller::getInstance()->sendCaptureSaved(
          m_id, QFileInfo(completePath).canonicalFilePath());
    } else {
        saveMessage += QObject::tr("Error trying to save as ") + completePath;
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

    QString suffix = ConfigHandler().getSaveAsFileExtension();
    QString defaultMimeType =
      QMimeDatabase().mimeTypeForFile("image" + suffix).name();
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
    QString savePath = FileNameHandler().absoluteSavePath();
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
          ShowSaveFileDialog(nullptr,
                             QObject::tr("Save screenshot"),
                             FileNameHandler().absoluteSavePath() +
                               ConfigHandler().getSaveAsFileExtension());
    }
    if (savePath == "") {
        return ok;
    }

    ok = capture.save(savePath);

    if (ok) {
        QString pathNoFile =
          savePath.left(savePath.lastIndexOf(QLatin1String("/")));

        ConfigHandler().setSavePath(pathNoFile);

        QString msg = QObject::tr("Capture saved as ") + savePath;

        if (config.copyPathAfterSaveEnabled()) {
            msg =
              QObject::tr("Capture is saved and copied to the clipboard as ") +
              savePath;
        }

        SystemNotification().sendMessage(msg, savePath);

        Controller::getInstance()->sendCaptureSaved(
          m_id, QFileInfo(savePath).canonicalFilePath());

        if (config.copyPathAfterSaveEnabled()) {
            QApplication::clipboard()->setText(savePath);
        }

    } else {
        QString msg = QObject::tr("Error trying to save as ") + savePath;
        QMessageBox saveErrBox(
          QMessageBox::Warning, QObject::tr("Save Error"), msg);
        saveErrBox.setWindowIcon(QIcon(":img/app/flameshot.svg"));
        saveErrBox.exec();
    }

    return ok;
}
