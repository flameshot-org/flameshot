// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2022 Dearsh Oberoi

#include "colorpickereditmode.h"
#include "src/utils/confighandler.h"
#include <QMouseEvent>
#include <QPainter>

ColorPickerEditMode::ColorPickerEditMode(QWidget* parent)
  : ColorPickerWidget(parent)
{}

void ColorPickerEditMode::mousePressEvent(QMouseEvent* e)
{
    if (e->button() == Qt::RightButton) {
        return;
    }

    for (int i = 1; i < m_colorList.size(); ++i) {
        if (m_colorAreaList.at(i).contains(e->pos())) {
            m_selectedIndex = i;
            update(m_colorAreaList.at(i) + QMargins(10, 10, 10, 10));
            update(m_colorAreaList.at(m_lastIndex) + QMargins(10, 10, 10, 10));
            m_lastIndex = i;
            emit colorSelected(m_selectedIndex);
            break;
        }
    }
}
