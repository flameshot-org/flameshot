// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "linetool.h"
#include <QPainter>

LineTool::LineTool(QObject* parent)
  : AbstractTwoPointTool(parent)
{
    m_supportsOrthogonalAdj = true;
    m_supportsDiagonalAdj = true;
}

QIcon LineTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor)
    return QIcon(iconPath(background) + "line.svg");
}

QString LineTool::name() const
{
    return tr("Line");
}

ToolType LineTool::nameID() const
{
    return ToolType::LINE;
}

QString LineTool::description() const
{
    return tr("Set the Line as the paint tool");
}

CaptureTool* LineTool::copy(QObject* parent)
{
    auto* tool = new LineTool(parent);
    copyParams(this, tool);
    return tool;
}

void LineTool::process(QPainter& painter, const QPixmap& pixmap)
{
    Q_UNUSED(pixmap)
    painter.setPen(QPen(color(), thickness()));
    painter.drawLine(points().first, points().second);
}

void LineTool::pressed(const CaptureContext& context)
{
    Q_UNUSED(context)
}
