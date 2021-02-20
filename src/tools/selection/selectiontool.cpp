// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "selectiontool.h"
#include <QPainter>

namespace {
#define PADDING_VALUE 2
}

SelectionTool::SelectionTool(QObject* parent)
  : AbstractTwoPointTool(parent)
{
    m_supportsDiagonalAdj = true;
}

bool SelectionTool::closeOnButtonPressed() const
{
    return false;
}

QIcon SelectionTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor);
    return QIcon(iconPath(background) + "square-outline.svg");
}
QString SelectionTool::name() const
{
    return tr("Rectangular Selection");
}

ToolType SelectionTool::nameID() const
{
    return ToolType::SELECTION;
}

QString SelectionTool::description() const
{
    return tr("Set Selection as the paint tool");
}

CaptureTool* SelectionTool::copy(QObject* parent)
{
    return new SelectionTool(parent);
}

void SelectionTool::process(QPainter& painter,
                            const QPixmap& pixmap,
                            bool recordUndo)
{
    if (recordUndo) {
        updateBackup(pixmap);
    }
    painter.setPen(QPen(m_color, m_thickness));
    painter.drawRect(QRect(m_points.first, m_points.second));
}

void SelectionTool::paintMousePreview(QPainter& painter,
                                      const CaptureContext& context)
{
    painter.setPen(QPen(context.color, context.thickness));
    painter.drawLine(context.mousePos, context.mousePos);
}

void SelectionTool::drawStart(const CaptureContext& context)
{
    m_color = context.color;
    m_thickness = context.thickness + PADDING_VALUE;
    m_points.first = context.mousePos;
    m_points.second = context.mousePos;
}

void SelectionTool::pressed(const CaptureContext& context)
{
    Q_UNUSED(context);
}
