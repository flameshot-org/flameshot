// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "colorpicker.h"
#include "src/utils/confighandler.h"
#include "src/utils/globalvalues.h"
#include <QMouseEvent>
#include <QPainter>

ColorPicker::ColorPicker(QWidget* parent)
  : ColorPickerWidget(parent)
{}

void ColorPicker::showEvent(QShowEvent* event)
{
    grabMouse();
}

void ColorPicker::hideEvent(QHideEvent* event)
{
    releaseMouse();
    emit colorSelected(m_colorList.at(m_selectedIndex));
}
