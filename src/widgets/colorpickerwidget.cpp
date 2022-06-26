// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2022 Dearsh Oberoi

#include "colorpickerwidget.h"
#include "src/utils/confighandler.h"
#include "src/utils/globalvalues.h"
#include <QMouseEvent>
#include <QPainter>

ColorPickerWidget::ColorPickerWidget(QWidget* parent)
  : QWidget(parent)
  , m_selectedIndex(1)
  , m_lastIndex(1)
{
    initColorPicker();
}

const QVector<QColor>& ColorPickerWidget::getDefaultSmallColorPalette()
{
    return defaultSmallColorPalette;
}

const QVector<QColor>& ColorPickerWidget::getDefaultLargeColorPalette()
{
    return defaultLargeColorPalette;
}

void ColorPickerWidget::paintEvent(QPaintEvent* e)
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

void ColorPickerWidget::repaint(int i, QPainter& painter)
{
    // draw the highlight when we have to draw the selected color
    if (i == m_selectedIndex) {
        auto c = QColor(m_uiColor);
        c.setAlpha(155);
        painter.setBrush(c);
        c.setAlpha(100);
        painter.setPen(c);
        QRect highlight = m_colorAreaList.at(i);
        const int highlightThickness = 6;

        // makes the highlight and color circles concentric
        highlight.moveTo(highlight.x() - (highlightThickness / 2),
                         highlight.y() - (highlightThickness / 2));

        highlight.setHeight(highlight.height() + highlightThickness);
        highlight.setWidth(highlight.width() + highlightThickness);
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

            painter.setPen(QColor(Qt::black));
        }
    }
}

void ColorPickerWidget::updateSelection(int index)
{
    m_selectedIndex = index;
    update(m_colorAreaList.at(index) + QMargins(10, 10, 10, 10));
    update(m_colorAreaList.at(m_lastIndex) + QMargins(10, 10, 10, 10));
    m_lastIndex = index;
}

void ColorPickerWidget::updateWidget()
{
    m_colorAreaList.clear();
    initColorPicker();
    update();
}

void ColorPickerWidget::initColorPicker()
{
    ConfigHandler config;
    m_colorList = config.userColors();
    m_colorAreaSize = GlobalValues::buttonBaseSize() * 0.6;
    // save the color values in member variables for faster access
    m_uiColor = config.uiColor();

    // extraSize represents the extra space needed for the highlight of the
    // selected color.
    const int extraSize = 6;
    const double slope = 3;
    double radius = slope * m_colorList.size() + GlobalValues::buttonBaseSize();
    setMinimumSize(radius * 2 + m_colorAreaSize + extraSize,
                   radius * 2 + m_colorAreaSize + extraSize);
    resize(radius * 2 + m_colorAreaSize + extraSize,
           radius * 2 + m_colorAreaSize + extraSize);
    double degree = (double)360 / m_colorList.size();
    double degreeAcum = 90;
    // this line is the radius of the circle which will be rotated to add
    // the color components.
    QLineF baseLine =
      QLineF(QPoint(radius + extraSize / 2, radius + extraSize / 2),
             QPoint(radius + extraSize / 2, extraSize / 2));

    for (int i = 0; i < m_colorList.size(); ++i) {
        m_colorAreaList.append(QRect(
          baseLine.x2(), baseLine.y2(), m_colorAreaSize, m_colorAreaSize));
        degreeAcum += degree;
        baseLine.setAngle(degreeAcum);
    }
}

QVector<QColor> ColorPickerWidget::defaultSmallColorPalette = {
    QColor(),      Qt::darkRed, Qt::red,  Qt::yellow,  Qt::green,
    Qt::darkGreen, Qt::cyan,    Qt::blue, Qt::magenta, Qt::darkMagenta
};

QVector<QColor> ColorPickerWidget::defaultLargeColorPalette = {
    QColor(),        Qt::white,     Qt::red,       Qt::green,     Qt::blue,
    Qt::black,       Qt::darkRed,   Qt::darkGreen, Qt::darkBlue,  Qt::darkGray,
    Qt::cyan,        Qt::magenta,   Qt::yellow,    Qt::lightGray, Qt::darkCyan,
    Qt::darkMagenta, Qt::darkYellow
};
