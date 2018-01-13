﻿// Copyright(c) 2017-2018 Alejandro Sirgo Rica & Contributors
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

#include "colorpicker.h"
#include <QPainter>
#include <QMouseEvent>
#include "src/utils/confighandler.h"


ColorPicker::ColorPicker(QWidget *parent) : QWidget(parent),
        m_colorAreaSize(18)
{
    setMouseTracking(true);
    // save the color values in member variables for faster access
    ConfigHandler config;
    m_uiColor = config.uiMainColorValue();
    m_drawColor = config.drawColorValue();
    // extraSize represents the extra space needed for the highlight of the
    // selected color.
    const int extraSize = 6;
    double radius = (colorList.size()*m_colorAreaSize/1.3)/(3.141592);
    resize(radius*2 + m_colorAreaSize + extraSize,
           radius*2 + m_colorAreaSize+ extraSize);
    double degree = 360 / (colorList.size());
    double degreeAcum = degree;
    // this line is the radius of the circle which will be rotated to add
    // the color components.
    QLineF baseLine = QLineF(QPoint(radius+extraSize/2, radius+extraSize/2),
                             QPoint(radius*2, radius));

    for (int i = 0; i<colorList.size(); ++i) {
        m_colorAreaList.append(QRect(baseLine.x2(), baseLine.y2(),
                                 m_colorAreaSize, m_colorAreaSize));
        baseLine.setAngle(degreeAcum);
        degreeAcum += degree;
    }
}

ColorPicker::~ColorPicker() {
    ConfigHandler().setDrawColor(m_drawColor);
}

QColor ColorPicker::drawColor() {
    return m_drawColor;
}

void ColorPicker::show() {
    grabMouse();
    QWidget::show();
}

void ColorPicker::hide() {
    releaseMouse();
    QWidget::hide();
}

void ColorPicker::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QVector<QRect> rects = handleMask();
    painter.setPen(QColor(Qt::black));
    for (int i = 0; i < rects.size(); ++i) {
        // draw the highlight when we have to draw the selected color
        if (m_drawColor == QColor(colorList.at(i))) {
            QColor c = QColor(m_uiColor);
            c.setAlpha(155);
            painter.setBrush(c);
            c.setAlpha(100);
            painter.setPen(c);
            QRect highlight = rects.at(i);
            highlight.moveTo(highlight.x() - 3, highlight.y() - 3);
            highlight.setHeight(highlight.height() + 6);
            highlight.setWidth(highlight.width() + 6);
            painter.drawRoundRect(highlight, 100, 100);
            painter.setPen(QColor(Qt::black));
        }
        painter.setBrush(QColor(colorList.at(i)));
        painter.drawRoundRect(rects.at(i), 100, 100);
    }
}

void ColorPicker::mouseMoveEvent(QMouseEvent *e) {
    for (int i = 0; i < colorList.size(); ++i) {
        if (m_colorAreaList.at(i).contains(e->pos())) {
            m_drawColor = colorList.at(i);
            update();
            break;
        }
    }
}

QVector<QRect> ColorPicker::handleMask() const {
    QVector<QRect> areas;
    for (const QRect &rect: m_colorAreaList) {
        areas.append(rect);
    }

    return areas;
}

QVector<Qt::GlobalColor> ColorPicker::colorList = {
    Qt::darkRed,
    Qt::red,
    Qt::yellow,
    Qt::green,
    Qt::darkGreen,
    Qt::cyan,
    Qt::blue,
    Qt::magenta,
    Qt::darkMagenta
};
