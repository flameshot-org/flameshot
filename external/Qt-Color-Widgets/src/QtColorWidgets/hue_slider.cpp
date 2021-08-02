/**
 * \file
 *
 * \author Mattia Basaglia
 *
 * \copyright Copyright (C) 2014 Calle Laakkonen
 * \copyright Copyright (C) 2013-2020 Mattia Basaglia
 * \copyright Copyright (C) 2017 caryoscelus
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "QtColorWidgets/hue_slider.hpp"

namespace color_widgets {

class HueSlider::Private
{
private:
    HueSlider *w;

public:
    qreal saturation = 1;
    qreal value = 1;
    qreal alpha = 1;

    Private(HueSlider *widget)
        : w(widget)
    {
        w->setRange(0, 359);
        connect(w, &QSlider::valueChanged, [this]{
            Q_EMIT w->colorHueChanged(w->colorHue());
            Q_EMIT w->colorChanged(w->color());
        });
        updateGradient();
    }

    void updateGradient()
    {
        static const double n_colors = 6;
        QGradientStops colors;
        colors.reserve(n_colors+1);
        for ( int i = 0; i <= n_colors; ++i )
            colors.append(QGradientStop(i/n_colors, QColor::fromHsvF(i/n_colors, saturation, value)));
        w->setColors(colors);
    }
};

HueSlider::HueSlider(QWidget *parent) :
    GradientSlider(parent), p(new Private(this))
{
}

HueSlider::HueSlider(Qt::Orientation orientation, QWidget *parent) :
    GradientSlider(orientation, parent), p(new Private(this))
{
}

HueSlider::~HueSlider()
{
	delete p;
}

qreal HueSlider::colorSaturation() const
{
    return p->saturation;
}

void HueSlider::setColorSaturation(qreal s)
{
    p->saturation = qBound(0.0, s, 1.0);
    p->updateGradient();
    Q_EMIT colorSaturationChanged(s);
}

qreal HueSlider::colorValue() const
{
    return p->value;
}

void HueSlider::setColorValue(qreal v)
{
    p->value = qBound(0.0, v, 1.0);
    p->updateGradient();
    Q_EMIT colorValueChanged(v);
}

qreal HueSlider::colorAlpha() const
{
    return p->alpha;
}

void HueSlider::setColorAlpha(qreal alpha)
{
    p->alpha = alpha;
    p->updateGradient();
    Q_EMIT colorAlphaChanged(alpha);
}

QColor HueSlider::color() const
{
    return QColor::fromHsvF(colorHue(), p->saturation, p->value, p->alpha);
}

void HueSlider::setColor(const QColor& color)
{
    p->saturation = color.saturationF();
    p->value = color.valueF();
    p->updateGradient();
    setColorHue(color.hueF());
    Q_EMIT colorValueChanged(p->alpha);
    Q_EMIT colorSaturationChanged(p->alpha);
}

void HueSlider::setFullColor(const QColor& color)
{
    p->alpha = color.alphaF();
    setColor(color);
    Q_EMIT colorAlphaChanged(p->alpha);
}

qreal HueSlider::colorHue() const
{
    if (maximum() == minimum())
        return 0;
    auto hue = qreal(value() - minimum()) / (maximum() - minimum());
    if (orientation() == Qt::Vertical)
        hue = 1 - hue;
    return hue;
}

void HueSlider::setColorHue(qreal colorHue)
{
    // TODO: consider supporting invertedAppearance?
    if (orientation() == Qt::Vertical)
        colorHue = 1 - colorHue;
    setValue(minimum()+colorHue*(maximum()-minimum()));
    Q_EMIT colorHueChanged(colorHue);
    Q_EMIT colorChanged(color());
}

} // namespace color_widgets
