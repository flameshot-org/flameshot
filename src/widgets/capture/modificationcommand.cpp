// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "modificationcommand.h"
#include <QPainter>

ModificationCommand::ModificationCommand(QPixmap* p, CaptureTool* t)
  : m_pixmap(p)
  , m_tool(t)
{
    setText(t->name());
}

void ModificationCommand::undo()
{
    m_tool->undo(*m_pixmap);
}

void ModificationCommand::redo()
{
    QPainter p(m_pixmap);
    p.setRenderHint(QPainter::Antialiasing);
    m_tool->process(p, *m_pixmap, true);
    if (m_tool->nameID() == ToolType::CIRCLECOUNT) {
        emit this->m_tool->requestAction(
          CaptureTool::Request::REQ_INCREMENT_CIRCLE_COUNT);
    }
}
