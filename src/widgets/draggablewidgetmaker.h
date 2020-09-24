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

#pragma once

#include <QEvent>
#include <QObject>
#include <QPoint>
#include <QWidget>

class DraggableWidgetMaker : public QObject
{
    Q_OBJECT
public:
    DraggableWidgetMaker(QObject* parent = nullptr);

    void makeDraggable(QWidget* widget);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    bool m_isPressing = false;
    bool m_isDragging = false;
    QPoint m_mouseMovePos;
    QPoint m_mousePressPos;
};
