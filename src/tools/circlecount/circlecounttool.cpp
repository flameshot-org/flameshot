// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "circlecounttool.h"
#include "colorutils.h"
#include <QPainter>
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#include "spdlog/cfg/env.h"
#include "spdlog/spdlog.h"

namespace {
#define PADDING_VALUE 2
}

CircleCountTool::CircleCountTool(QObject* parent)
  : AbstractTwoPointTool(parent)
{
    m_count = 0;
}

QIcon CircleCountTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor);
    return QIcon(iconPath(background) + "circlecount-outline.svg");
}
QString CircleCountTool::name() const
{
    return tr("Circle Counter");
}

ToolType CircleCountTool::nameID() const
{
    return ToolType::CIRCLECOUNT;
}

QString CircleCountTool::description() const
{
    return tr("Add an autoincrementing counter bubble");
}

CaptureTool* CircleCountTool::copy(QObject* parent)
{
    return new CircleCountTool(parent);
}

void CircleCountTool::process(QPainter& painter,
                              const QPixmap& pixmap,
                              bool recordUndo)
{
    if (recordUndo) {
        updateBackup(pixmap);
    }
    auto orig_pen = painter.pen();
    QBrush orig_brush = painter.brush();
    painter.setBrush(m_color);

    int bubble_size = m_thickness;
    painter.drawEllipse(m_points.first, bubble_size, bubble_size);
    QRect textRect = QRect(m_points.first.x() - bubble_size / 2,
                           m_points.first.y() - bubble_size / 2,
                           bubble_size,
                           bubble_size);
    auto orig_font = painter.font();
    auto new_font = orig_font;
    auto fontSize = bubble_size;
    new_font.setPixelSize(fontSize);
    new_font.setBold(true);
    painter.setFont(new_font);

    QRect bRect =
      painter.boundingRect(textRect, Qt::AlignCenter, QString::number(m_count));

    while (bRect.width() > textRect.width()) {
        fontSize--;
        if (fontSize == 0) {
            break;
        }
        new_font.setPixelSize(fontSize);
        painter.setFont(new_font);
        bRect = painter.boundingRect(
          textRect, Qt::AlignCenter, QString::number(m_count));
    }

    if (ColorUtils::colorIsDark(m_color)) {
        painter.setPen(Qt::white);
    } else {
        painter.setPen(Qt::black);
    }

    painter.drawText(textRect, Qt::AlignCenter, QString::number(m_count));
    painter.setFont(orig_font);
    painter.setBrush(orig_brush);
    painter.setPen(orig_pen);
}

void CircleCountTool::paintMousePreview(QPainter& painter,
                                        const CaptureContext& context)
{
    m_thickness = context.thickness + PADDING_VALUE;
    if (m_thickness < 15) {
        m_thickness = 15;
    }

    // Thickness for pen is *2 to range from radius to diameter to match the
    // ellipse draw function
    painter.setPen(
      QPen(context.color, m_thickness * 2, Qt::SolidLine, Qt::RoundCap));
    painter.drawLine(context.mousePos,
                     { context.mousePos.x() + 1, context.mousePos.y() + 1 });
}

void CircleCountTool::drawStart(const CaptureContext& context)
{
    m_color = context.color;
    m_thickness = context.thickness + PADDING_VALUE;
    if (m_thickness < 15) {
        m_thickness = 15;
    }
    m_points.first = context.mousePos;
    m_count = context.circleCount;
    emit requestAction(REQ_INCREMENT_CIRCLE_COUNT);
}

void CircleCountTool::pressed(const CaptureContext& context)
{
    Q_UNUSED(context);
}

void CircleCountTool::undo()
{
    emit requestAction(CaptureTool::Request::REQ_DECREMENT_CIRCLE_COUNT);
}

void CircleCountTool::redo()
{
    emit requestAction(CaptureTool::Request::REQ_INCREMENT_CIRCLE_COUNT);
}