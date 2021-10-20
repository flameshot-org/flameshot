// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "penciltool.h"
#include <QPainter>

PencilTool::PencilTool(QObject* parent)
  : AbstractPathTool(parent)
{}

QIcon PencilTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor)
    return QIcon(iconPath(background) + "pencil.svg");
}
QString PencilTool::name() const
{
    return tr("Pencil");
}

CaptureTool::Type PencilTool::type() const
{
    return CaptureTool::TYPE_PENCIL;
}

QString PencilTool::description() const
{
    return tr("Set the Pencil as the paint tool");
}

CaptureTool* PencilTool::copy(QObject* parent)
{
    auto* tool = new PencilTool(parent);
    copyParams(this, tool);
    return tool;
}

void PencilTool::process(QPainter& painter, const QPixmap& pixmap)
{
    Q_UNUSED(pixmap)
    painter.setPen(QPen(m_color, size()));
    painter.drawPolyline(m_points.data(), m_points.size());
}

void PencilTool::paintMousePreview(QPainter& painter,
                                   const CaptureContext& context)
{
    painter.setPen(QPen(context.color, context.toolSize + 2));
    painter.drawLine(context.mousePos, context.mousePos);
}

void PencilTool::drawStart(const CaptureContext& context)
{
    m_color = context.color;
    onSizeChanged(context.toolSize);
    m_points.append(context.mousePos);
    m_pathArea.setTopLeft(context.mousePos);
    m_pathArea.setBottomRight(context.mousePos);
}

void PencilTool::pressed(CaptureContext& context)
{
    Q_UNUSED(context)
}
