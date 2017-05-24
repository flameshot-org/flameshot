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

#include "capturemodification.h"
#include <QColor>

// CaptureModification is a single modification in the screenshot drawn
// by the user.

CaptureModification::CaptureModification(const Button::Type t, const QPoint p,
                                         const QColor c) : m_color(c), m_type(t)
{
    m_coords.append(p);
    if (m_type == Button::Type::circle || m_type == Button::Type::rectangle
         || m_type == Button::Type::arrow || m_type == Button::Type::line ||
            m_type == Button::Type::marker) {
        m_coords.append(p);
    }
}

CaptureModification::CaptureModification() {

}

Button::Type CaptureModification::getType() const {
    return m_type;
}

QColor CaptureModification::getColor() const {
    return m_color;
}

QVector<QPoint> CaptureModification::getPoints() const {
    return m_coords;
}
// addPoint adds a point to the vector of points
void CaptureModification::addPoint(const QPoint p) {
    if (m_type == Button::Type::circle || m_type == Button::Type::rectangle
         || m_type == Button::Type::arrow || m_type == Button::Type::line ||
            m_type == Button::Type::marker) {
        m_coords[1] = p;
    } else {
        m_coords.append(p);
    }
}
