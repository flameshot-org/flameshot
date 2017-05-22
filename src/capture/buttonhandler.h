// Copyright 2017 Alejandro Sirgo Rica
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

#ifndef BUTTONHANDLER_H
#define BUTTONHANDLER_H

#include "button.h"
#include <QVector>

class Button;
class QRect;
class QPoint;

class ButtonHandler {
public:
    ButtonHandler(const QVector<Button*>&);
    ButtonHandler();

    void hide();
    void show();

    bool isVisible() const;
    size_t size() const;

    void updatePosition(const QRect &selection, const QRect &limits);
    void setButtons(const QVector<Button*>);

private:
    QVector<QPoint> getHPoints(const QPoint &center, const int elements) const;
    QVector<QPoint> getVPoints(const QPoint &center, const int elements) const;

    QVector<Button*> m_vectorButtons;
    int m_distance;
};

#endif // BUTTONHANDLER_H
