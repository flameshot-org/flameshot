// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include "src/tools/capturetool.h"
#include <QUndoCommand>

class ModificationCommand : public QUndoCommand
{
public:
    ModificationCommand(QPixmap*, CaptureTool*);

    virtual void undo() override;
    virtual void redo() override;

private:
    QPixmap* m_pixmap;
    QScopedPointer<CaptureTool> m_tool;
};
