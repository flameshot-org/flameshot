// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "screenshotsaver.h"
#include "abstractlogger.h"
#include "src/core/flameshot.h"
#include "src/core/flameshotdaemon.h"
#include "src/utils/confighandler.h"
#include "src/utils/filenamehandler.h"
#include "src/utils/globalvalues.h"
#include "utils/desktopinfo.h"

#if USE_WAYLAND_CLIPBOARD
#include <KSystemClipboard>
#endif

#include <QApplication>
#include <QBuffer>
#include <QClipboard>
#include <QFileDialog>
#include <QMessageBox>
#include <QMimeData>
#include <QStandardPaths>
#include <qimagewriter.h>
#include <qmimedatabase.h>
#if defined(Q_OS_MACOS)
#include "src/widgets/capture/capturewidget.h"
#endif

bool saveToFilesystem(const QPixmap& capture,
                      const QString& path,
                      const QString& messagePrefix)
{
    QString completePath = FileNameHandler().properScreenshotPath(
      path, ConfigHandler().saveAsFileExtension());
    QFile file{ completePath };
    file.open(QIODevice::WriteOnly);
    bool okay = capture.save(&file);
    QString saveMessage = messagePrefix;
    QString notificationPath = completePath;
    if (!saveMessage.isEmpty()) {
        saveMessage += " ";
    }

    if (okay) {
        saveMessage += QObject::tr("Capture saved as ") + completePath;
        AbstractLogger::info().attachNotificationPath(notificationPath)
          << saveMessage;
    } else {
        saveMessage += QObject::tr("Error trying to save as ") + completePath;
        if (file.error() != QFile::NoError) {
            saveMessage += ": " + file.errorString();
        }
        notificationPath = "";
        AbstractLogger::error().attachNotificationPath(notificationPath)
          << saveMessage;
    }

    return okay;
}

QString ShowSaveFileDialog(const QString& title, const QString& directory)
{
    QFileDialog dialog(nullptr, title, directory);
    dialog.setAcceptMode(QFileDialog::AcceptSave);

    // Build string list of supported image formats
    QStringList mimeTypeList;
    foreach (auto mimeType, QImageWriter::supportedMimeTypes()) {
        // HEIF is meant for videos and it causes a glitch on MacOS
        // because the native dialog lumps together heic and heif
        if (mimeType != "image/heif") {
            mimeTypeList.append(mimeType);
        }
    }
    dialog.setMimeTypeFilters(mimeTypeList);

    QString suffix = ConfigHandler().saveAsFileExtension();
    if (suffix.isEmpty()) {
        suffix = "png";
    }
    QString defaultMimeType =
      QMimeDatabase().mimeTypeForFile("image." + suffix).name();
    dialog.selectMimeTypeFilter(defaultMimeType);
    dialog.setDefaultSuffix(suffix);
    if (dialog.exec() == QDialog::Accepted) {
        return dialog.selectedFiles().constFirst();
    } else {
        return {};
    }
}

void saveToClipboardMime(const QPixmap& capture, const QString& imageType)
{
    QByteArray array;
    QBuffer buffer{ &array };
    QImageWriter imageWriter{ &buffer, imageType.toUpper().toUtf8() };
    imageWriter.write(capture.toImage());

    QPixmap formattedPixmap;
    bool isLoaded =
      formattedPixmap.loadFromData(reinterpret_cast<uchar*>(array.data()),
                                   array.size(),
                                   imageType.toUpper().toUtf8());
    if (isLoaded) {

        auto* mimeData = new QMimeData();

#ifdef USE_WAYLAND_CLIPBOARD
        mimeData->setImageData(formattedPixmap.toImage());
        mimeData->setData(QStringLiteral("x-kde-force-image-copy"),
                          QByteArray());
        KSystemClipboard::instance()->setMimeData(mimeData,
                                                  QClipboard::Clipboard);
#else
        mimeData->setData("image/" + imageType, array);
        QApplication::clipboard()->setMimeData(mimeData);
#endif

    } else {
        AbstractLogger::error()
          << QObject::tr("Error while saving to clipboard");
    }
}

// If data is saved to the clipboard before the notification is sent via
// dbus, the application freezes.
void saveToClipboard(const QPixmap& capture)
{
    // If we are able to properly save the file, save the file and copy to
    // clipboard.
    if ((ConfigHandler().saveAfterCopy()) &&
        (!ConfigHandler().savePath().isEmpty())) {
        saveToFilesystem(capture,
                         ConfigHandler().savePath(),
                         QObject::tr("Capture saved to clipboard."));
    } else {
        AbstractLogger() << QObject::tr("Capture saved to clipboard.");
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

bool saveToFilesystemGUI(const QPixmap& capture)
{
    bool okay = false;
    ConfigHandler config;
    QString defaultSavePath = ConfigHandler().savePath();
    if (defaultSavePath.isEmpty() || !QDir(defaultSavePath).exists() ||
        !QFileInfo(defaultSavePath).isWritable()) {
        defaultSavePath =
          QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    }
    QString savePath = FileNameHandler().properScreenshotPath(
      defaultSavePath, ConfigHandler().saveAsFileExtension());
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
        savePath = ShowSaveFileDialog(QObject::tr("Save screenshot"), savePath);
    }
    if (savePath == "") {
        return okay;
    }

    QFile file{ savePath };
    file.open(QIODevice::WriteOnly);

    okay = capture.save(&file);

    if (okay) {
        QString pathNoFile =
          savePath.left(savePath.lastIndexOf(QLatin1String("/")));

        ConfigHandler().setSavePath(pathNoFile);

        QString msg = QObject::tr("Capture saved as ") + savePath;
        AbstractLogger().attachNotificationPath(savePath) << msg;

        if (config.copyPathAfterSave()) {
            FlameshotDaemon::copyToClipboard(
              savePath, QObject::tr("Path copied to clipboard as ") + savePath);
        }

    } else {
        QString msg = QObject::tr("Error trying to save as ") + savePath;

        if (file.error() != QFile::NoError) {
            msg += ": " + file.errorString();
        }

        QMessageBox saveErrBox(
          QMessageBox::Warning, QObject::tr("Save Error"), msg);
        saveErrBox.setWindowIcon(QIcon(GlobalValues::iconPath()));
        saveErrBox.exec();
    }

    return okay;
}
