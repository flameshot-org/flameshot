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

#pragma once

#include <QString>
#include <QPixmap>
#include <QVariant>

class CaptureRequest {
public:
    enum CaptureMode {
        FULLSCREEN_MODE,
        GRAPHICAL_MODE,
        SCREEN_MODE,
    };

    enum ExportTask {
        NO_TASK = 0,
        CLIPBOARD_SAVE_TASK = 1,
        FILESYSTEM_SAVE_TASK = 2,
    };

    CaptureRequest(CaptureMode mode,
                   const uint delay = 0,
                   const QString &path = QLatin1String(""),
                   const QVariant &data = QVariant(),
                   ExportTask tasks = NO_TASK);

    void setStaticID(uint id);

    uint id() const;
    uint delay() const;
    QString path() const;
    QVariant data() const;
    CaptureMode captureMode() const;

    void addTask(ExportTask task);
    void exportCapture(const QPixmap &p);

private:
    CaptureMode m_mode;
    uint m_delay;
    QString m_path;
    ExportTask m_tasks;
    QVariant m_data;

    bool m_forcedID;
    uint m_id;
};

using eTask = CaptureRequest::ExportTask;

inline eTask operator|(const eTask &a, const eTask &b) {
    return static_cast<eTask>(static_cast<int>(a) | static_cast<int>(b));
}

inline eTask operator&(const eTask &a, const eTask &b) {
    return static_cast<eTask>(static_cast<int>(a) & static_cast<int>(b));
}

inline eTask& operator|=(eTask &a, const eTask &b) {
    a = static_cast<eTask>(static_cast<int>(a) | static_cast<int>(b));
    return a;
}

