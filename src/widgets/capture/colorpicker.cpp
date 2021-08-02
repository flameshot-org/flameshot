// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "colorpicker.h"
#include "src/utils/confighandler.h"
#include "src/utils/globalvalues.h"
#include <QMouseEvent>
#include <QPainter>

ColorPicker::ColorPicker(QWidget* parent)
  : QWidget(parent)
  , m_selectedIndex(0)
  , m_lastIndex(0)
{
    ConfigHandler config;
    m_colorList = config.userColors();
    m_colorAreaSize = GlobalValues::buttonBaseSize() * 0.6;
    setMouseTracking(true);
    // save the color values in member variables for faster access
    m_uiColor = config.uiColor();
    QColor drawColor = config.drawColor();
    for (int i = 0; i < m_colorList.size(); ++i) {
        if (m_colorList.at(i) == drawColor) {
            m_selectedIndex = i;
            m_lastIndex = i;
            break;
        }
    }
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

void ColorPicker::paintEvent(QPaintEvent* e)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QColor(Qt::black));

    for (int i = 0; i < m_colorAreaList.size(); ++i) {
        if (e->region().contains(m_colorAreaList.at(i))) {
            painter.setClipRegion(e->region());
            repaint(i, painter);
        }
    }
}

void ColorPicker::repaint(int i, QPainter& painter)
{
    // draw the highlight when we have to draw the selected color
    if (i == m_selectedIndex) {
        QColor c = QColor(m_uiColor);
        c.setAlpha(155);
        painter.setBrush(c);
        c.setAlpha(100);
        painter.setPen(c);
        QRect highlight = m_colorAreaList.at(i);
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
        painter.drawRoundedRect(m_colorAreaList.at(i), 100, 100);
    } else {
        // draw rainbow (part) for custom color
        QRect lastRect = m_colorAreaList.at(i);
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
            painter.drawRoundedRect(lastRect, 100, 100);

            // set next color, circle geometry
            h += fHStep;
            lastRect.setX(lastRect.x() + nStep);
            lastRect.setY(lastRect.y() + nStep);
            lastRect.setHeight(lastRect.height() - nStep);
            lastRect.setWidth(lastRect.width() - nStep);
        }
    }
}

void ColorPicker::mouseMoveEvent(QMouseEvent* e)
{
    for (int i = 0; i < m_colorList.size(); ++i) {
        if (m_colorAreaList.at(i).contains(e->pos())) {
            m_selectedIndex = i;
            update(m_colorAreaList.at(i) + QMargins(10, 10, 10, 10));
            update(m_colorAreaList.at(m_lastIndex) + QMargins(10, 10, 10, 10));
            m_lastIndex = i;
            break;
        }
    }
}

void ColorPicker::showEvent(QShowEvent* event)
{
    grabMouse();
}

void ColorPicker::hideEvent(QHideEvent* event)
{
    releaseMouse();
    emit colorSelected(m_colorList.at(m_selectedIndex));
}
