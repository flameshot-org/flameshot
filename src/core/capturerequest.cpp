// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "capturerequest.h"
#include "confighandler.h"
#include "controller.h"
#include "imguruploader.h"
#include "pinwidget.h"
#include "src/utils/screenshotsaver.h"
#include "src/widgets/imguruploaddialog.h"
#include "systemnotification.h"
#include <QApplication>
#include <QClipboard>
#include <QDateTime>
#include <QVector>
#include <stdexcept>

CaptureRequest::CaptureRequest(CaptureRequest::CaptureMode mode,
                               const uint delay,
                               const QVariant& data,
                               CaptureRequest::ExportTask tasks)
  : m_mode(mode)
  , m_delay(delay)
  , m_tasks(tasks)
  , m_data(data)
{}

CaptureRequest::CaptureMode CaptureRequest::captureMode() const
{
    return m_mode;
}

uint CaptureRequest::delay() const
{
    return m_delay;
}

QString CaptureRequest::path() const
{
    return m_path;
}

QVariant CaptureRequest::data() const
{
    return m_data;
}

CaptureRequest::ExportTask CaptureRequest::tasks() const
{
    return m_tasks;
}

void CaptureRequest::addTask(CaptureRequest::ExportTask task)
{
    if (task == SAVE) {
        throw std::logic_error("SAVE task must be added using addSaveTask");
    }
    m_tasks |= task;
}

void CaptureRequest::removeTask(ExportTask task)
{
    ((int&)m_tasks) &= ~task;
}

void CaptureRequest::addSaveTask(const QString& path)
{
    m_tasks |= SAVE;
    m_path = path;
}

void CaptureRequest::addPinTask(const QRect& pinWindowGeometry)
{
    m_tasks |= PIN;
    m_pinWindowGeometry = pinWindowGeometry;
}

void CaptureRequest::exportCapture(const QPixmap& capture)
{
    if (m_tasks & SAVE) {
        if (m_path.isEmpty()) {
            ScreenshotSaver().saveToFilesystemGUI(capture);
        } else {
            ScreenshotSaver().saveToFilesystem(capture, m_path);
        }
    }

    if (m_tasks & COPY) {
        ScreenshotSaver().saveToClipboard(capture);
    }

    if (m_tasks & PIN) {
        QWidget* widget = new PinWidget(capture, m_pinWindowGeometry);
        widget->show();
        widget->activateWindow();
        if (m_mode == SCREEN_MODE || m_mode == FULLSCREEN_MODE) {
            SystemNotification().sendMessage(
              QObject::tr("Full screen screenshot pinned to screen"));
        }
    }

    if (m_tasks & UPLOAD) {
        if (!ConfigHandler().uploadWithoutConfirmation()) {
            ImgurUploadDialog* dialog = new ImgurUploadDialog();
            if (dialog->exec() == QDialog::Rejected) {
                return;
            }
        }
        ImgurUploader* widget = new ImgurUploader(capture);
        widget->show();
        widget->activateWindow();
        // NOTE: lambda can't capture 'this' because it might be destroyed later
        ExportTask tasks = m_tasks;
        QObject::connect(
          widget, &ImgurUploader::uploadOk, [widget, tasks](const QUrl& url) {
              if (ConfigHandler().copyAndCloseAfterUpload()) {
                  if (!(tasks & COPY)) {
                      QApplication::clipboard()->setText(url.toString());
                      SystemNotification().sendMessage(
                        QObject::tr("URL copied to clipboard."));
                      widget->close();
                  } else {
                      widget->showPostUploadDialog();
                  }
              } else {
                  widget->showPostUploadDialog();
              }
          });
    }
}
