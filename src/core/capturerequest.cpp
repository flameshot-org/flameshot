// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "capturerequest.h"
#include "src/utils/screenshotsaver.h"
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
  , m_forcedID(false)
  , m_id(0)
{}

void CaptureRequest::setStaticID(uint id)
{
    m_forcedID = true;
    m_id = id;
}

uint CaptureRequest::id() const
{
    if (m_forcedID) {
        return m_id;
    }

    uint id = 0;
    QVector<uint> v;
    v << qHash(m_mode) << qHash(m_delay * QDateTime::currentMSecsSinceEpoch())
      << qHash(m_path) << qHash(m_tasks) << m_data.toInt();
    for (uint i : v) {
        id ^= i + 0x9e3779b9 + (id << 6) + (id >> 2);
    }
    return id;
}

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
    if (task == SAVE_TASK) {
        throw std::logic_error("SAVE_TASK must be added using addSaveTask");
    }
    m_tasks |= task;
}

void CaptureRequest::addSaveTask(const QString& path)
{
    m_tasks |= SAVE_TASK;
    m_path = path;
}

void CaptureRequest::exportCapture(const QPixmap& p)
{
    if ((m_tasks & ExportTask::SAVE_TASK) != ExportTask::NO_TASK) {
        if (m_path.isEmpty()) {
            ScreenshotSaver(m_id).saveToFilesystemGUI(p);
        } else {
            ScreenshotSaver(m_id).saveToFilesystem(p, m_path);
        }
    }

    if ((m_tasks & ExportTask::COPY_TASK) != ExportTask::NO_TASK) {
        ScreenshotSaver().saveToClipboard(p);
    }
}
