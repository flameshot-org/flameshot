// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include "capturetoolobjects.h"
#include <QUndoCommand>

class CaptureWidget;

class ModificationCommand : public QUndoCommand
{
public:
    ModificationCommand(CaptureWidget* captureWidget,
                        const CaptureToolObjects& captureToolObjects,
                        const CaptureToolObjects& captureToolObjectsBackup);

    virtual void undo() override;
    virtual void redo() override;

private:
    CaptureToolObjects m_captureToolObjects;
    CaptureToolObjects m_captureToolObjectsBackup;
    CaptureWidget* m_captureWidget;
};
