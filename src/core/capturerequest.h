// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QPixmap>
#include <QString>
#include <QVariant>

class CaptureRequest
{
public:
    enum CaptureMode
    {
        FULLSCREEN_MODE,
        GRAPHICAL_MODE,
        SCREEN_MODE,
    };

    enum ExportTask
    {
        NO_TASK = 0,
        CLIPBOARD_SAVE_TASK = 1,
        FILESYSTEM_SAVE_TASK = 2,
    };

    CaptureRequest(CaptureMode mode,
                   const uint delay = 0,
                   const QString& path = QLatin1String(""),
                   const QVariant& data = QVariant(),
                   ExportTask tasks = NO_TASK);

    void setStaticID(uint id);

    uint id() const;
    uint delay() const;
    QString path() const;
    QVariant data() const;
    CaptureMode captureMode() const;

    void addTask(ExportTask task);
    void exportCapture(const QPixmap& p);

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

inline eTask operator|(const eTask& a, const eTask& b)
{
    return static_cast<eTask>(static_cast<int>(a) | static_cast<int>(b));
}

inline eTask operator&(const eTask& a, const eTask& b)
{
    return static_cast<eTask>(static_cast<int>(a) & static_cast<int>(b));
}

inline eTask& operator|=(eTask& a, const eTask& b)
{
    a = static_cast<eTask>(static_cast<int>(a) | static_cast<int>(b));
    return a;
}
