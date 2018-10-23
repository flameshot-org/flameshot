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
#include <QApplication>

BlurTool::BlurTool(QObject *parent) : AbstractTwoPointTool(parent) {

}

QIcon BlurTool::icon(const QColor &background, bool inEditor) const {
    Q_UNUSED(inEditor);
    return QIcon(iconPath(background) + "blur.svg");
}
QString BlurTool::name() const {
    return tr("Blur");
}

QString BlurTool::nameID() {
    return QLatin1String("");
}

QString BlurTool::description() const {
    return tr("Set Blur as the paint tool");
}

CaptureTool* BlurTool::copy(QObject *parent) {
    return new BlurTool(parent);
}

void BlurTool::process(QPainter &painter, const QPixmap &pixmap, bool recordUndo) {
    if (recordUndo) {
        updateBackup(pixmap);
    }
    QPoint &p0 = m_points.first;
    QPoint &p1 = m_points.second;
    auto pixelRatio = pixmap.devicePixelRatio();

    QRect selection = QRect(p0, p1).normalized();
    QRect selectionScaled = QRect(p0 * pixelRatio, p1 * pixelRatio).normalized();

    QGraphicsBlurEffect *blur = new QGraphicsBlurEffect;
    blur->setBlurRadius(10);
    QGraphicsPixmapItem *item = new QGraphicsPixmapItem (
                pixmap.copy(selectionScaled));
    item->setGraphicsEffect(blur);

    QGraphicsScene scene;
    scene.addItem(item);

    scene.render(&painter, selection, QRectF());
    blur->setBlurRadius(12);
    scene.render(&painter, selection, QRectF());
    scene.render(&painter, selection, QRectF());
}

void BlurTool::paintMousePreview(QPainter &painter, const CaptureContext &context) {
    Q_UNUSED(context);
    Q_UNUSED(painter);
}

void BlurTool::drawStart(const CaptureContext &context) {
    m_thickness = context.thickness;
    m_points.first = context.mousePos;
    m_points.second = context.mousePos;
}

void BlurTool::pressed(const CaptureContext &context) {
    Q_UNUSED(context);
}
