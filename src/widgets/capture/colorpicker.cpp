/// Copyright(c) 2017-2019 Alejandro Sirgo Rica & Contributors
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
#include "src/utils/confighandler.h"
#include "src/utils/globalvalues.h"
#include <QMouseEvent>
#include <QPainter>

ColorPicker::ColorPicker(QWidget* parent)
  : QWidget(parent)
{
    ConfigHandler config;
    m_colorList = config.getUserColors();
    m_colorAreaSize = GlobalValues::buttonBaseSize() * 0.6;
    setMouseTracking(true);
    // save the color values in member variables for faster access
    m_uiColor = config.uiMainColorValue();
    m_drawColor = config.drawColorValue();
    // extraSize represents the extra space needed for the highlight of the
    // selected color.
    const int extraSize = 6;
    double radius = (m_colorList.size() * m_colorAreaSize / 1.3) / 3.141592;
    resize(radius * 2 + m_colorAreaSize + extraSize,
           radius * 2 + m_colorAreaSize + extraSize);
    double degree = 360 / (m_colorList.size());
    double degreeAcum = degree;
    // this line is the radius of the circle which will be rotated to add
    // the color components.
    QLineF baseLine =
      QLineF(QPoint(radius + extraSize / 2, radius + extraSize / 2),
             QPoint(radius * 2, radius));

    for (int i = 0; i < m_colorList.size(); ++i) {
        m_colorAreaList.append(QRect(
          baseLine.x2(), baseLine.y2(), m_colorAreaSize, m_colorAreaSize));
        baseLine.setAngle(degreeAcum);
        degreeAcum += degree;
    }
}

QColor ColorPicker::drawColor()
{
    return m_drawColor;
}

void ColorPicker::show()
{
    grabMouse();
    QWidget::show();
}

void ColorPicker::hide()
{
    releaseMouse();
    QWidget::hide();
}

void ColorPicker::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QVector<QRect> rects = handleMask();
    painter.setPen(QColor(Qt::black));
    for (int i = 0; i < rects.size(); ++i) {
        // draw the highlight when we have to draw the selected color
        if (m_drawColor == QColor(m_colorList.at(i))) {
            QColor c = QColor(m_uiColor);
            c.setAlpha(155);
            painter.setBrush(c);
            c.setAlpha(100);
            painter.setPen(c);
            QRect highlight = rects.at(i);
            highlight.moveTo(highlight.x() - 3, highlight.y() - 3);
            highlight.setHeight(highlight.height() + 6);
            highlight.setWidth(highlight.width() + 6);
            painter.drawRoundedRect(highlight, 100, 100);
            painter.setPen(QColor(Qt::black));
        }

        // draw available colors
        if (m_colorList.at(i).isValid()) {
            // draw preset color
            painter.setBrush(QColor(m_colorList.at(i)));
            painter.drawRoundRect(rects.at(i), 100, 100);
        } else {
            // draw rainbow (part) for custom color
            QRect lastRect = rects.at(i);
            int nStep = 1;
            int nSteps = lastRect.height() / nStep;
            // 0.02 - start rainbow color, 0.33 - end rainbow color from range:
            // 0.0 - 1.0
            float h = 0.02;
            for (int radius = nSteps; radius > 0; radius -= nStep * 2) {
                // calculate color
                float fHStep = (0.33 - h) / (nSteps / nStep / 2);
                QColor color = QColor::fromHslF(h, 0.95, 0.5);

                // set color and draw circle
                painter.setPen(color);
                painter.setBrush(color);
                painter.drawRoundRect(lastRect, 100, 100);

                // set next color, circle geometry
                h += fHStep;
                lastRect.setX(lastRect.x() + nStep);
                lastRect.setY(lastRect.y() + nStep);
                lastRect.setHeight(lastRect.height() - nStep);
                lastRect.setWidth(lastRect.width() - nStep);
            }
        }
    }
}

void ColorPicker::mouseMoveEvent(QMouseEvent* e)
{
    for (int i = 0; i < m_colorList.size(); ++i) {
        if (m_colorAreaList.at(i).contains(e->pos())) {
            m_drawColor = m_colorList.at(i);
            emit colorSelected(m_drawColor);
            update();
            break;
        }
    }
}

QVector<QRect> ColorPicker::handleMask() const
{
    QVector<QRect> areas;
    for (const QRect& rect : m_colorAreaList) {
        areas.append(rect);
    }
    return areas;
}
