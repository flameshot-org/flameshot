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

#include "extendedslider.h"

ExtendedSlider::ExtendedSlider(QWidget *parent)
    : QSlider(parent)
{
    connect(this, &ExtendedSlider::valueChanged,
            this, &ExtendedSlider::updateTooltip);
    connect(this, &ExtendedSlider::sliderMoved,
            this, &ExtendedSlider::fireTimer);
    m_timer.setSingleShot(true);
    connect(&m_timer, &QTimer::timeout,
            this, &ExtendedSlider::modificationsEnded);
}

int ExtendedSlider::mappedValue(int min, int max) {
    qreal progress =
            ((value() - minimum())) / static_cast<qreal>(maximum() - minimum());
    return min + (max - min) * progress;
}

void ExtendedSlider::setMapedValue(int min, int val, int max) {
    qreal progress = ((val - min) + 1) / static_cast<qreal>(max - min);
    int value = minimum() + (maximum() - minimum()) * progress;
    setValue(value);
}

void ExtendedSlider::updateTooltip() {
    setToolTip(QString::number(value())+"%");
}

void ExtendedSlider::fireTimer() {
    m_timer.start(500);
}
