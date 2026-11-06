// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "rectangletool.h"

#include <QPainter>
#include <QPainterPath>
#include <cmath>

RectangleTool::RectangleTool(QObject* parent)
  : AbstractTwoPointTool(parent)
{
    m_supportsDiagonalAdj = true;
}

QIcon RectangleTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor)
    return QIcon(iconPath(background) + "square.svg");
}
QString RectangleTool::name() const
{
    return tr("Rectangle");
}

CaptureTool::Type RectangleTool::type() const
{
    return CaptureTool::TYPE_RECTANGLE;
}

QString RectangleTool::description() const
{
    return tr("Set the Rectangle as the paint tool");
}

CaptureTool* RectangleTool::copy(QObject* parent)
{
    auto* tool = new RectangleTool(parent);
    copyParams(this, tool);
    return tool;
}

void RectangleTool::process(QPainter& painter, const QPixmap& pixmap)
{
    Q_UNUSED(pixmap)
    QPen orig_pen = painter.pen();
    QBrush orig_brush = painter.brush();
    painter.setPen(
      QPen(color(), size(), Qt::SolidLine, Qt::SquareCap, Qt::RoundJoin));
    painter.setBrush(QBrush(color()));
    if (size() == 0) {
        painter.drawRect(QRect(points().first, points().second));
    } else {
        QPainterPath path;
        int offset =
          size() <= 1 ? 1 : static_cast<int>(round(size() / 2 + 0.5));
        path.addRoundedRect(
          QRectF(
            std::min(points().first.x(), points().second.x()) - offset,
            std::min(points().first.y(), points().second.y()) - offset,
            std::abs(points().first.x() - points().second.x()) + offset * 2,
            std::abs(points().first.y() - points().second.y()) + offset * 2),
          size(),
          size());
        painter.fillPath(path, color());
    }
    painter.setPen(orig_pen);
    painter.setBrush(orig_brush);
}

void RectangleTool::drawStart(const CaptureContext& context)
{
    AbstractTwoPointTool::drawStart(context);
    onSizeChanged(context.toolSize);
}

void RectangleTool::pressed(CaptureContext& context)
{
    Q_UNUSED(context)
}

WhiteBackgroundRectangleTool::WhiteBackgroundRectangleTool(QObject* parent)
  : RectangleTool(parent)
{}

QIcon WhiteBackgroundRectangleTool::icon(const QColor& background,
                                         bool inEditor) const
{
    Q_UNUSED(background)
    Q_UNUSED(inEditor)
    QPixmap iconPixmap(24, 24);
    iconPixmap.fill(Qt::transparent);
    QPainter painter(&iconPixmap);
    painter.setPen(QPen(Qt::red, 2));
    painter.setBrush(Qt::white);
    painter.drawRect(QRect(3, 3, 18, 18));
    return QIcon(iconPixmap);
}

QString WhiteBackgroundRectangleTool::name() const
{
    return tr("White background rectangle");
}

QString WhiteBackgroundRectangleTool::description() const
{
    return tr("Draw an opaque white rectangle with a colored outline");
}

CaptureTool::Type WhiteBackgroundRectangleTool::type() const
{
    return CaptureTool::TYPE_WHITE_BACKGROUND_RECTANGLE;
}

CaptureTool* WhiteBackgroundRectangleTool::copy(QObject* parent)
{
    auto* tool = new WhiteBackgroundRectangleTool(parent);
    copyParams(this, tool);
    return tool;
}

void WhiteBackgroundRectangleTool::process(QPainter& painter,
                                           const QPixmap& pixmap)
{
    Q_UNUSED(pixmap)
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setPen(
      QPen(color(), 2, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
    painter.setBrush(Qt::white);
    painter.drawRect(QRect(points().first, points().second).normalized());
    painter.restore();
}
