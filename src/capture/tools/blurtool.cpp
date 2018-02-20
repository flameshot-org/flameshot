// Copyright(c) 2017-2018 Alejandro Sirgo Rica & Contributors
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

#include "blurtool.h"
#include <QPainter>
#include <QGraphicsBlurEffect>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>

BlurTool::BlurTool(QObject *parent) : CaptureTool(parent) {

}

int BlurTool::id() const {
    return 0;
}

bool BlurTool::isSelectable() const {
    return true;
}

QString BlurTool::iconName() const {
    return "blur.png";
}

QString BlurTool::name() const {
    return tr("Blur");
}

QString BlurTool::description() const {
    return tr("Sets the Blur as the paint tool");
}

CaptureTool::ToolWorkType BlurTool::toolType() const {
    return TYPE_LINE_DRAWER;
}
#include <QApplication>
void BlurTool::processImage(
        QPainter &painter,
        const QVector<QPoint> &points,
        const QColor &color,
        const int thickness)
{
    Q_UNUSED(color);
    Q_UNUSED(thickness);
    QPoint p0 = points[0];
    QPoint p1 = points[1];

    QRect selection = QRect(p0, p1).normalized();
    QPixmap *refPixmap = dynamic_cast<QPixmap*>(painter.device());

    QGraphicsBlurEffect *blur = new QGraphicsBlurEffect;
    blur->setBlurRadius(10);
    QGraphicsPixmapItem *item = new QGraphicsPixmapItem (
                refPixmap->copy(selection));
    item->setGraphicsEffect(blur);

    QGraphicsScene scene;
    scene.addItem(item);

    scene.render(&painter, selection, QRectF());
    blur->setBlurRadius(12);
    scene.render(&painter, selection, QRectF());
    scene.render(&painter, selection, QRectF());
}

void BlurTool::onPressed() {
}
