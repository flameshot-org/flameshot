// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "extendedslider.h"

ExtendedSlider::ExtendedSlider(QWidget* parent)
  : QSlider(parent)
{
    connect(this,
            &ExtendedSlider::valueChanged,
            this,
            &ExtendedSlider::updateTooltip);
    connect(
      this, &ExtendedSlider::sliderMoved, this, &ExtendedSlider::fireTimer);
    m_timer.setSingleShot(true);
    connect(
      &m_timer, &QTimer::timeout, this, &ExtendedSlider::modificationsEnded);
}

int ExtendedSlider::mappedValue(int min, int max)
{
    qreal progress =
      ((value() - minimum())) / static_cast<qreal>(maximum() - minimum());
    return min + (max - min) * progress;
}

void ExtendedSlider::setMapedValue(int min, int val, int max)
{
    qreal progress = ((val - min) + 1) / static_cast<qreal>(max - min);
    int value = minimum() + (maximum() - minimum()) * progress;
    setValue(value);
}

void ExtendedSlider::updateTooltip()
{
    setToolTip(QString::number(value()) + "%");
}

void ExtendedSlider::fireTimer()
{
    m_timer.start(500);
}
