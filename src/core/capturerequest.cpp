// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "capturerequest.h"
#include "confighandler.h"
#include "flameshot.h"
#include "imgupload/imguploadermanager.h"
#include "pinwidget.h"
#include "src/utils/screenshotsaver.h"
#include "src/widgets/imguploaddialog.h"
#include "systemnotification.h"
#include <QApplication>
#include <QClipboard>
#include <QDateTime>
#include <QVector>
#include <stdexcept>
#include <utility>

CaptureRequest::CaptureRequest(CaptureRequest::CaptureMode mode,
                               const uint delay,
                               QVariant data,
                               CaptureRequest::ExportTask tasks)
  : m_mode(mode)
  , m_delay(delay)
  , m_tasks(tasks)
  , m_data(std::move(data))
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

QRect CaptureRequest::initialSelection() const
{
    return m_initialSelection;
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

void CaptureRequest::setInitialSelection(const QRect& selection)
{
    m_initialSelection = selection;
}
