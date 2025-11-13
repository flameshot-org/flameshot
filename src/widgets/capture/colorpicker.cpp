// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "colorpicker.h"
#include "src/utils/confighandler.h"
#include "src/utils/globalvalues.h"
#include <cmath>
#include <QMouseEvent>
#include <QPainter>
#include <QPoint>
#include <QSize>

ColorPicker::ColorPicker(QWidget* parent)
  : ColorPickerWidget(parent)
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
    emit colorSelected(m_colorList.at(m_selectedIndex));
}

void ColorPicker::mouseMoveEvent(QMouseEvent* e)
{
    const size_t colsize = m_colorList.size();
    const QSize  middle  = this->size() * 0.5;
    const QPoint center  = -(this->pos() - this->mapToParent(e->pos())
        + QPoint(middle.width(), middle.height()));
    const double arclength = 2.0 * M_PI / colsize;

    // Computes the new index based on the orientation of the mouse
    // from the middle of the circle of colors.
    //
    // This is done with the following equation:
    //
    //  index = angle_made_on_unit_circle / angle_per_color + color_shift
    //
    //  Where andle_made_on_unit_circle is from [0, 2pi] given std::atan2,
    //  and angle_per_color simply is 2pi / number_of_colors.
    //
    //  The color_shift is 3 * number_of_colors / 4 since the index
    //  of the first color is -3pi / 2 (in the screen coordinates,
    //  so pi/2 in normal coordinates) and the colors are being drawn
    //  counter clockwise.
    m_selectedIndex = (
       int(
            (std::atan2(-center.y(), center.x()) + arclength / 2) / arclength
            + 3 * colsize / 4.0
        )
    ) % colsize;

    update(m_colorAreaList.at(m_selectedIndex) + QMargins(10, 10, 10, 10));
    update(m_colorAreaList.at(m_lastIndex) + QMargins(10, 10, 10, 10));
    m_lastIndex = m_selectedIndex;
}

void ColorPicker::showEvent(QShowEvent* event)
{
    grabMouse();
}

void ColorPicker::hideEvent(QHideEvent* event)
{
    releaseMouse();
}
