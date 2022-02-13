// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2022 Dearsh Oberoi

#include "colorpickereditmode.h"

#include <QMouseEvent>
#include <QPainter>

ColorPickerEditMode::ColorPickerEditMode(QWidget* parent)
  : ColorPickerWidget(parent)
{
    m_isPressing = false;
    m_isDragging = false;
    installEventFilter(this);
}

bool ColorPickerEditMode::eventFilter(QObject* obj, QEvent* event)
{
    auto widget = static_cast<QWidget*>(obj);

    switch (event->type()) {
        case QEvent::MouseButtonPress: {
            auto mouseEvent = static_cast<QMouseEvent*>(event);

            if (mouseEvent->button() == Qt::LeftButton) {
                m_mousePressPos = mouseEvent->pos();
                m_mouseMovePos = m_mousePressPos;

                for (int i = 1; i < m_colorList.size(); ++i) {
                    if (m_colorAreaList.at(i).contains(m_mousePressPos)) {
                        m_isPressing = true;
                        m_draggedPresetInitialPos =
                          m_colorAreaList[i].topLeft();
                        m_selectedIndex = i;
                        update(m_colorAreaList.at(i) +
                               QMargins(10, 10, 10, 10));
                        update(m_colorAreaList.at(m_lastIndex) +
                               QMargins(10, 10, 10, 10));
                        m_lastIndex = i;
                        emit colorSelected(m_selectedIndex);
                        break;
                    }
                }
            }
        } break;
        case QEvent::MouseMove: {
            auto mouseEvent = static_cast<QMouseEvent*>(event);

            if (m_isPressing) {
                QPoint eventPos = mouseEvent->pos();
                QPoint diff = eventPos - m_mouseMovePos;
                m_colorAreaList[m_selectedIndex].translate(diff);
                widget->update();

                if (!m_isDragging) {
                    QPoint totalMovedDiff = eventPos - m_mousePressPos;
                    if (totalMovedDiff.manhattanLength() > 3) {
                        m_isDragging = true;
                    }
                }

                m_mouseMovePos = eventPos;
            }
        } break;
        case QEvent::MouseButtonRelease: {
            m_isPressing = false;
            if (m_isDragging) {
                QPoint draggedPresetCenter =
                  m_colorAreaList[m_selectedIndex].center();
                m_isDragging = false;

                bool swapped = false;

                for (int i = 1; i < m_colorList.size(); ++i) {
                    if (i != m_selectedIndex &&
                        m_colorAreaList.at(i).contains(draggedPresetCenter)) {
                        // swap colors
                        QColor temp = m_colorList[i];
                        m_colorList[i] = m_colorList[m_selectedIndex];
                        m_colorList[m_selectedIndex] = temp;
                        m_config.setUserColors(m_colorList);

                        m_colorAreaList[m_selectedIndex].moveTo(
                          m_draggedPresetInitialPos);
                        m_selectedIndex = i;
                        widget->update();
                        m_lastIndex = i;
                        emit presetsSwapped(m_selectedIndex);
                        swapped = true;
                        break;
                    }
                }

                if (!swapped) {
                    m_colorAreaList[m_selectedIndex].moveTo(
                      m_draggedPresetInitialPos);
                    widget->update();
                }
            }
        } break;
        default:
            break;
    }

    return QObject::eventFilter(obj, event);
}
