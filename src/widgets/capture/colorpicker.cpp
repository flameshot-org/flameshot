// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "colorpicker.h"
#include "src/utils/confighandler.h"
#include "src/utils/globalvalues.h"
#include <QMouseEvent>
#include <QPainter>

ColorPicker::ColorPicker(QPixmap* p, QWidget* parent)
  : ColorPickerWidget(parent)
  , m_pixmap(p)
{
    setMouseTracking(true);

    ConfigHandler config;
    QColor drawColor = config.drawColor();
    for (int i = 0; i < m_colorList.size(); ++i) {
        if (m_colorList.at(i) == drawColor) {
            m_selectedIndex = i;
            m_lastIndex = i;
            break;
        }
    }
}
void ColorPicker::setNewColor()
{
    if (!m_colorList.at(m_selectedIndex).isValid() && m_selectedIndex == 1) {
        startColorGrab();
    } else {
        emit colorSelected(m_colorList.at(m_selectedIndex));
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

void ColorPicker::startColorGrab()
{
    m_colorGrabber = new ColorGrabWidget(m_pixmap);
    connect(m_colorGrabber,
            &ColorGrabWidget::colorGrabbed,
            this,
            &ColorPicker::onColorGrabFinished);

    m_colorGrabber->startGrabbing();
}

void ColorPicker::onColorGrabFinished()
{
    emit colorSelected(m_colorGrabber->color());
}

void ColorPicker::showEvent(QShowEvent* event)
{
    grabMouse();
}

void ColorPicker::hideEvent(QHideEvent* event)
{
    releaseMouse();
}
