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

#include "colorutils.h"

inline qreal getColorLuma(const QColor &c) {
    return 0.30 * c.redF() + 0.59 * c.greenF() + 0.11 * c.blueF();
}

bool ColorUtils::colorIsDark(const QColor &c) {
    bool isWhite = false;
    if (getColorLuma(c) <= 0.60) {
        isWhite = true;
    }
    return isWhite;
}

QColor ColorUtils::contrastColor(const QColor &c) {
    int change = colorIsDark(c) ? 30 : -45;

    return QColor(qBound(0, c.red() + change, 255),
                  qBound(0, c.green() + change, 255),
                  qBound(0, c.blue() + change, 255));
}

