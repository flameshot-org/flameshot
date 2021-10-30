// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "capturerequest.h"
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
