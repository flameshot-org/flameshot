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
                                            const QString& directory,
                                            const QString& filter)
{
#if defined(Q_WS_WIN) || defined(Q_WS_MAC)
    return QFileDialog::getSaveFileName(parent, title, directory, filter);
#else
    QFileDialog dialog(parent, title, directory, filter);
    if (parent) {
        dialog.setWindowModality(Qt::WindowModal);
    }

    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.selectNameFilter(ConfigHandler().getSaveAsFileExtension());
    if (dialog.exec() == QDialog::Accepted) {

        ConfigHandler().setSaveAsFileExtension(dialog.selectedNameFilter());
        QString file_name = dialog.selectedFiles().first();
        QFileInfo info(file_name);

        if ((dialog.selectedNameFilter() == defaultFilter)) {
            if (info.suffix().isEmpty()) { // change to png if no suffix given,
                                           // otherwise leave it as it is
                file_name = info.filePath() + QLatin1String(".") + "png";
                ;
            }
        } else if (!dialog.selectedNameFilter()
                      .isEmpty()) { // if selected suffix from menu is not an
                                    // empty entry
            QString selectedExtension =
              dialog.selectedNameFilter().section('.', -1);
            selectedExtension.remove(QChar(')'));
            file_name =
              info.path() + QLatin1String("/") + info.baseName() +
              QLatin1String(".") +
              selectedExtension; // recreate full filename with chosen suffix
        }
        return file_name;
    } else {
        return QString();
    }
#endif // Q_WS_MAC || Q_WS_WIN
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
        savePath = ShowSaveFileDialog(
          nullptr,
          QObject::tr("Save screenshot"),
          FileNameHandler().absoluteSavePath(),
          QString(pngFilter + separator + bmpFilter + separator + jpgFilter +
                  separator + defaultFilter));
    }
    if (savePath == "") {
        QString msg = QObject::tr("Saving canceled");
        QMessageBox saveInfoBox(
          QMessageBox::Information, QObject::tr("Save canceled"), msg);
        saveInfoBox.setWindowIcon(QIcon(":img/app/flameshot.svg"));
        saveInfoBox.exec();
        return ok;
    } else if (!savePath.endsWith(QLatin1String(".png"), Qt::CaseInsensitive) &&
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
