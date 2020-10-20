// Copyright(c) 2017-2019 Alejandro Sirgo Rica & Contributors
//
// This file is part of Flameshot.
//
//     Flameshot is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     Flameshot is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with Flameshot.  If not, see <http://www.gnu.org/licenses/>.

#include "pixelatetool.h"
#include <QApplication>
#include <QGraphicsBlurEffect>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QImage>
#include <QPainter>
#include <cassert>

PixelateTool::PixelateTool(QObject* parent)
  : AbstractTwoPointTool(parent)
{}

QIcon PixelateTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor);
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
    return new PixelateTool(parent);
}

void PixelateTool::process(QPainter& painter,
                           const QPixmap& pixmap,
                           bool recordUndo)
{

    if (recordUndo) {
        updateBackup(pixmap);
    }

    QPoint& p0 = m_points.first;
    QPoint& p1 = m_points.second;
    QRect selection = QRect(p0, p1).normalized();

    // If thickness is less than 1, use old blur process
    if (m_thickness <= 1) {
        auto pixelRatio = pixmap.devicePixelRatio();

        QRect selectionScaled =
          QRect(p0 * pixelRatio, p1 * pixelRatio).normalized();

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
        int width = selection.width() * (0.5 / qMax(1, m_thickness));
        int height = selection.height() * (0.5 / qMax(1, m_thickness));
        QSize size = QSize(qMax(width, 1), qMax(height, 1));

        QPixmap t = pixmap.copy(selection);
        t = t.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        t = t.scaled(selection.width(), selection.height());
        painter.drawImage(selection, t.toImage());
    }
}

void PixelateTool::paintMousePreview(QPainter& painter,
                                     const CaptureContext& context)
{
    Q_UNUSED(context);
    Q_UNUSED(painter);
}

void PixelateTool::drawStart(const CaptureContext& context)
{
    m_thickness = context.thickness;
    m_points.first = context.mousePos;
    m_points.second = context.mousePos;
}

void PixelateTool::pressed(const CaptureContext& context)
{
    Q_UNUSED(context);
}
