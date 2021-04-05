// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "modificationcommand.h"
#include "capturewidget.h"

ModificationCommand::ModificationCommand(
  CaptureWidget* captureWidget,
  const CaptureToolObjects& captureToolObjects)
  : m_captureWidget(captureWidget)
  , m_firstRun(true)
{
    m_captureToolObjects = captureToolObjects;
}

void ModificationCommand::undo()
{
    m_captureWidget->setCaptureToolObjects(m_captureToolObjects);
}

void ModificationCommand::redo()
{
    if (!m_firstRun) {
        m_captureWidget->setCaptureToolObjects(m_captureToolObjects);
    }
    m_firstRun = false;
}
