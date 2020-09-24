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

// Based on https://stackoverflow.com/a/53135675/964478

#pragma once

#include "capture/capturebutton.h"
#include <QPushButton>

class OrientablePushButton : public CaptureButton
{
    Q_OBJECT
public:
    enum Orientation
    {
        Horizontal,
        VerticalTopToBottom,
        VerticalBottomToTop
    };

    OrientablePushButton(QWidget* parent = nullptr);
    OrientablePushButton(const QString& text, QWidget* parent = nullptr);
    OrientablePushButton(const QIcon& icon,
                         const QString& text,
                         QWidget* parent = nullptr);

    QSize sizeHint() const;

    OrientablePushButton::Orientation orientation() const;
    void setOrientation(const OrientablePushButton::Orientation& orientation);

protected:
    void paintEvent(QPaintEvent* event);

private:
    Orientation m_orientation = Horizontal;
};
