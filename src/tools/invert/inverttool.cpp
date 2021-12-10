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
    return QRect(points().first, points().second).normalized();
}

CaptureTool* InvertTool::copy(QObject* parent)
{
    auto* tool = new InvertTool(parent);
    copyParams(this, tool);
    return tool;
}

void InvertTool::process(QPainter& painter, const QPixmap& pixmap)
{
    QRect selection = boundingRect().intersected(pixmap.rect());
    auto pixelRatio = pixmap.devicePixelRatio();
    QRect selectionScaled = QRect(selection.topLeft() * pixelRatio,
                                  selection.bottomRight() * pixelRatio);

    // Invert selection
    QPixmap inv = pixmap.copy(selectionScaled);
    QImage img = inv.toImage();
    img.invertPixels();

    painter.drawImage(selection, img);
}

void InvertTool::drawSearchArea(QPainter& painter, const QPixmap& pixmap)
{
    Q_UNUSED(pixmap)
    painter.fillRect(boundingRect(), QBrush(Qt::black));
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
