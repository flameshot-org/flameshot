// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "inverttool.h"
#include <QApplication>
#include <QGraphicsBlurEffect>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QImage>
#include <QPainter>
#include <QPixmap>

InvertTool::InvertTool(QObject* parent)
  : AbstractTwoPointTool(parent)
{}

QIcon InvertTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor)
    return QIcon(iconPath(background) + "invert.svg");
}

QString InvertTool::name() const
{
    return tr("Invert");
}

CaptureTool::Type InvertTool::type() const
{
    return CaptureTool::TYPE_INVERT;
}

QString InvertTool::description() const
{
    return tr("Set Inverter as the paint tool");
}

QRect InvertTool::boundingRect() const
{
    return QRect(std::min(points().first.x(), points().second.x()),
                 std::min(points().first.y(), points().second.y()),
                 std::abs(points().first.x() - points().second.x()),
                 std::abs(points().first.y() - points().second.y()))
      .normalized();
}

CaptureTool* InvertTool::copy(QObject* parent)
{
    auto* tool = new InvertTool(parent);
    copyParams(this, tool);
    return tool;
}

void InvertTool::process(QPainter& painter, const QPixmap& pixmap)
{
    QPoint p0 = points().first;
    QPoint p1 = points().second;
    QRect selection = QRect(p0, p1).normalized();

    // Invert selection
    QPixmap inv = pixmap.copy(selection);
    QImage img = inv.toImage();
    img.invertPixels();

    painter.drawImage(selection, img);
}

void InvertTool::drawSearchArea(QPainter& painter, const QPixmap& pixmap)
{
    Q_UNUSED(pixmap)
    painter.fillRect(std::min(points().first.x(), points().second.x()),
                     std::min(points().first.y(), points().second.y()),
                     std::abs(points().first.x() - points().second.x()),
                     std::abs(points().first.y() - points().second.y()),
                     QBrush(Qt::black));
}

void InvertTool::paintMousePreview(QPainter& painter,
                                   const CaptureContext& context)
{
    Q_UNUSED(context)
    Q_UNUSED(painter)
}

void InvertTool::pressed(CaptureContext& context)
{
    Q_UNUSED(context)
}
