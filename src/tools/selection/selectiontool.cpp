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
    Q_UNUSED(inEditor)
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
    SelectionTool* tool = new SelectionTool(parent);
    copyParams(this, tool);
    return tool;
}

void SelectionTool::process(QPainter& painter, const QPixmap& pixmap)
{
    Q_UNUSED(pixmap)
    painter.setPen(
      QPen(color(), thickness(), Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
    painter.drawRect(QRect(points().first, points().second));
}

void SelectionTool::pressed(const CaptureContext& context)
{
    Q_UNUSED(context)
}
