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

#include <QByteArray>
#include <QDebug>
#include <QImageWriter>
#include <QPixmap>
#include <QProcess>
#include <QTemporaryFile>
#include <QTimer>
#include <QWidget>

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

    bool okay;
    QString saveExtension;
    saveExtension = QFileInfo(completePath).suffix().toLower();
    if (saveExtension == "jpg" || saveExtension == "jpeg") {
        okay = capture.save(&file, nullptr, ConfigHandler().jpegQuality());
    } else {
        okay = capture.save(&file);
    }

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
    for (const auto& mimeType : QImageWriter::supportedMimeTypes()) {
        // image/heif has several aliases and they cause glitch in save dialog
        // It is necessary to keep the image/heif (otherwise HEIF plug-in from
        // kimageformats will not work) but the aliases could be filtered out.
        if (mimeType != "image/heic" && mimeType != "image/heic-sequence" &&
            mimeType != "image/heif-sequence") {
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

void saveJpegToClipboardMacOS(const QPixmap& capture)
{
    // Convert QPixmap to JPEG data
    QByteArray jpegData;
    QBuffer buffer(&jpegData);
    buffer.open(QIODevice::WriteOnly);

    QImageWriter imageWriter(&buffer, "jpeg");

    // Set JPEG quality to whatever is in settings
    imageWriter.setQuality(ConfigHandler().jpegQuality());
    if (!imageWriter.write(capture.toImage())) {
        qWarning() << "Failed to write image to JPEG format.";
        return;
    }

    // Save JPEG data to a temporary file
    QTemporaryFile tempFile;
    if (!tempFile.open()) {
        qWarning() << "Failed to open temporary file for writing.";
        return;
    }
    tempFile.write(jpegData);
    tempFile.close();

    // Use osascript to copy the contents of the file to clipboard
    QProcess process;
    QString script =
      QString("set the clipboard to (read (POSIX file \"%1\") as «class PNGf»)")
        .arg(tempFile.fileName());
    process.start("osascript", QStringList() << "-e" << script);
    if (!process.waitForFinished()) {
        qWarning() << "Failed to execute AppleScript.";
    }

    // Clean up
    tempFile.remove();
}

class LazyImageMimeData : public QMimeData  
{  
public:  
    LazyImageMimeData(const QPixmap& pixmap, const QString& format)  
        : m_pixmap(pixmap)  
        , m_format(format)  
    {  
    }  
  
    QStringList formats() const override  
    {  
        return { "image/" + m_format, "application/x-qt-image" };  
    }  
  
protected:  
    QVariant retrieveData(const QString& mimeType, QMetaType type) const override  
    {  
        if (mimeType == "application/x-qt-image") {  
            return m_pixmap.toImage();  
        }  
        if (mimeType == "image/" + m_format) {  
            QByteArray ba;  
            QBuffer buffer(&ba);  
            buffer.open(QIODevice::WriteOnly);  
            m_pixmap.toImage().save(&buffer, m_format.toUpper().toLatin1());  
            return ba;  
        }  
        return QMimeData::retrieveData(mimeType, type);  
    }  
  
private:  
    QPixmap m_pixmap;  
    QString m_format;  
};

void saveToClipboardMime(const QPixmap& capture, const QString& imageType)
{
#ifdef USE_WAYLAND_CLIPBOARD
    // GNOME + Wayland Workaround  
    DesktopInfo desktopInfo;  
    if (desktopInfo.waylandDetected() &&  
        desktopInfo.windowManager() == DesktopInfo::GNOME) {  
        QWidget helperWindow;  
        helperWindow.setWindowFlags(Qt::Window | Qt::FramelessWindowHint |  
                                    Qt::WindowStaysOnTopHint);  
        helperWindow.setAttribute(Qt::WA_TranslucentBackground);  
        helperWindow.setFixedSize(1, 1);  
    
        QEventLoop eventLoop;  
    
        QTimer::singleShot(50, [&]() {  
            auto* mimeData = new LazyImageMimeData(capture, imageType);  
            QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);  
            QTimer::singleShot(250, &eventLoop, &QEventLoop::quit);  
        });  
    
        helperWindow.show();  
        eventLoop.exec();  
        return;  
    }  
#endif
    QByteArray array;
    QBuffer buffer{ &array };
    QImageWriter imageWriter{ &buffer, imageType.toUpper().toUtf8() };
    if (imageType == "jpeg") {
        imageWriter.setQuality(ConfigHandler().jpegQuality());
    }
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
#ifdef Q_OS_MAC
        saveJpegToClipboardMacOS(capture);
#else
        saveToClipboardMime(capture, "jpeg");
#endif
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

    QString saveExtension;
    saveExtension = QFileInfo(savePath).suffix().toLower();
    if (saveExtension == "jpg" || saveExtension == "jpeg") {
        okay = capture.save(&file, nullptr, ConfigHandler().jpegQuality());
    } else {
        okay = capture.save(&file);
    }

    if (okay) {
        // Don't use QDir::separator() here, as Qt internally always uses '/'
        QString pathNoFile = savePath.left(savePath.lastIndexOf('/'));

        ConfigHandler().setSavePath(pathNoFile);

        QString msg = QObject::tr("Capture saved as ") + savePath;
        AbstractLogger().attachNotificationPath(savePath) << msg;

        if (config.copyPathAfterSave()) {
#ifdef Q_OS_WIN
            savePath.replace('/', '\\');
#endif
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
