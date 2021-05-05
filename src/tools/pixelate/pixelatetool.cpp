// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "pixelatetool.h"
#include <QApplication>
#include <QGraphicsBlurEffect>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QImage>
#include <QPainter>

PixelateTool::PixelateTool(QObject* parent)
  : AbstractTwoPointTool(parent)
{}

QIcon PixelateTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor)
    return QIcon(iconPath(background) + "pixelate.svg");
}
QString PixelateTool::name() const
{
    return tr("Pixelate");
}

ToolType PixelateTool::nameID() const
{
    return ToolType::PIXELATE;
}

QString PixelateTool::description() const
{
    return tr("Set Pixelate as the paint tool");
}

CaptureTool* PixelateTool::copy(QObject* parent)
{
    auto* tool = new PixelateTool(parent);
    copyParams(this, tool);
    return tool;
}

void PixelateTool::process(QPainter& painter, const QPixmap& pixmap)
{
    QPoint p0 = points().first;
    QPoint p1 = points().second;
    QRect selection = QRect(p0, p1).normalized();
    auto pixelRatio = pixmap.devicePixelRatio();
    QRect selectionScaled =
      QRect(p0 * pixelRatio, p1 * pixelRatio).normalized();

    // If thickness is less than 1, use old blur process
    if (thickness() <= 1) {
        QGraphicsBlurEffect* blur = new QGraphicsBlurEffect;
        blur->setBlurRadius(10);
        QGraphicsPixmapItem* item =
          new QGraphicsPixmapItem(pixmap.copy(selectionScaled));
        item->setGraphicsEffect(blur);

        QGraphicsScene scene;
        scene.addItem(item);

        scene.render(&painter, selection, QRectF());
        blur->setBlurRadius(12);
        // multiple repeat for make blur effect stronger
        scene.render(&painter, selection, QRectF());

    } else {
        int width = static_cast<int>(selection.width() *
                                     (0.5 / qMax(1, thickness() + 1)));
        int height = static_cast<int>(selection.height() *
                                      (0.5 / qMax(1, thickness() + 1)));
        QSize size = QSize(qMax(width, 1), qMax(height, 1));

        QPixmap t = pixmap.copy(selectionScaled);
        t = t.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        t = t.scaled(selection.width(), selection.height());
        painter.drawImage(selection, t.toImage());
    }
}

void PixelateTool::drawSearchArea(QPainter& painter, const QPixmap& pixmap)
{
    Q_UNUSED(pixmap)
    painter.fillRect(std::min(points().first.x(), points().second.x()),
                     std::min(points().first.y(), points().second.y()),
                     std::abs(points().first.x() - points().second.x()),
                     std::abs(points().first.y() - points().second.y()),
                     QBrush(Qt::black));
}

void PixelateTool::paintMousePreview(QPainter& painter,
                                     const CaptureContext& context)
{
    Q_UNUSED(context)
    Q_UNUSED(painter)
}

void PixelateTool::pressed(const CaptureContext& context)
{
    Q_UNUSED(context)
}

void PixelateTool::drawObjectSelection(QPainter& painter)
{
    QRect rect = QRect(std::min(points().first.x(), points().second.x()),
                       std::min(points().first.y(), points().second.y()),
                       std::abs(points().first.x() - points().second.x()),
                       std::abs(points().first.y() - points().second.y()));
    drawObjectSelectionRect(painter, rect);
}
