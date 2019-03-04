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

#include "capturerequest.h"
#include "src/utils/screenshotsaver.h"
#include <QVector>
#include <QDateTime>

CaptureRequest::CaptureRequest(CaptureRequest::CaptureMode mode,
                               const uint delay, const QString &path,
                               const QVariant &data,
                               CaptureRequest::ExportTask tasks) :
    m_mode(mode), m_delay(delay), m_path(path), m_tasks(tasks),
    m_data(data), m_forcedID(false), m_id(0)
{

}

void CaptureRequest::setStaticID(uint id) {
    m_forcedID = true;
    m_id = id;
}

uint CaptureRequest::id() const {
    if (m_forcedID) {
        return m_id;
    }

    uint id = 0;
    QVector<uint>v;
    v << qHash(m_mode) << qHash(m_delay * QDateTime::currentMSecsSinceEpoch())
      << qHash(m_path) << qHash(m_tasks) << m_data.toInt();
    for(uint i : v) {
        id ^= i + 0x9e3779b9 + (id << 6) + (id >> 2);
    }
    return id;
}

CaptureRequest::CaptureMode CaptureRequest::captureMode() const {
    return m_mode;
}

uint CaptureRequest::delay() const {
    return m_delay;
}

QString CaptureRequest::path() const {
    return m_path;
}

QVariant CaptureRequest::data() const {
    return m_data;
}

void CaptureRequest::addTask(CaptureRequest::ExportTask task) {
    m_tasks |= task;
}

void CaptureRequest::exportCapture(const QPixmap &p) {
    if ((m_tasks & ExportTask::FILESYSTEM_SAVE_TASK) != ExportTask::NO_TASK) {
        if (m_path.isEmpty()) {
            ScreenshotSaver().saveToFilesystemGUI(p);
        } else {
            ScreenshotSaver().saveToFilesystem(p, m_path);
        }
    }

    if ((m_tasks & ExportTask::CLIPBOARD_SAVE_TASK) != ExportTask::NO_TASK) {
        ScreenshotSaver().saveToClipboard(p);
    }

}
