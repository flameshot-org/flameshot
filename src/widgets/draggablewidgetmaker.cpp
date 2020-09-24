// Copyright(c) 2017-2019 Alejandro Sirgo Rica & Contributors
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

#include "draggablewidgetmaker.h"
#include <QMouseEvent>

DraggableWidgetMaker::DraggableWidgetMaker(QObject* parent)
  : QObject(parent)
{}

void DraggableWidgetMaker::makeDraggable(QWidget* widget)
{
    widget->installEventFilter(this);
}

bool DraggableWidgetMaker::eventFilter(QObject* obj, QEvent* event)
{
    auto widget = static_cast<QWidget*>(obj);

    // based on https://stackoverflow.com/a/12221360/964478
    switch (event->type()) {
        case QEvent::MouseButtonPress: {
            auto mouseEvent = static_cast<QMouseEvent*>(event);

            m_isPressing = false;
            m_isDragging = false;
            if (mouseEvent->button() == Qt::LeftButton) {
                m_isPressing = true;
                m_mousePressPos = mouseEvent->globalPos();
                m_mouseMovePos = m_mousePressPos;
            }
        } break;
        case QEvent::MouseMove: {
            auto mouseEvent = static_cast<QMouseEvent*>(event);

            if (m_isPressing) {
                QPoint widgetPos = widget->mapToGlobal(widget->pos());
                QPoint eventPos = mouseEvent->globalPos();
                QPoint diff = eventPos - m_mouseMovePos;
                QPoint newPos = widgetPos + diff;

                widget->move(widget->mapFromGlobal(newPos));

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
            if (m_isDragging) {
                m_isPressing = false;
                m_isDragging = false;
                event->ignore();
                return true;
            }
        } break;
    }

    return QObject::eventFilter(obj, event);
}
