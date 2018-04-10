// Copyright(c) 2017-2018 Alejandro Sirgo Rica & Contributors
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

#include <QRect>
#include <QPoint>
#include <QPixmap>
#include <QPainter>

struct CaptureContext {
    // screenshot with modifications
    QPixmap screenshot;
    // unmodified screenshot
    QPixmap origScreenshot;
    // Selection area
    QRect selection;
    // Widget dimensions
    QRect widgetDimensions;
    // Selected tool color
    QColor color;
    // Path where the content has to be saved
    QString savePath;
    // Ofset of the capture widget based on the system's screen (top-left)
    QPoint widgetOffset;
    // Mouse position inside the widget
    QPoint mousePos;
    // Value of the desired thickness
    int thickness;
    // Mode of the capture widget
    bool fullscreen;

    QPixmap selectedScreenshotArea() const ;
};
