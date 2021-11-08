/**
 * \file
 *
 * \author Mattia Basaglia
 *
 * \copyright Copyright (C) 2014 Calle Laakkonen
 * \copyright Copyright (C) 2013-2020 Mattia Basaglia
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
#ifndef HUE_SLIDER_HPP
#define HUE_SLIDER_HPP

#include "gradient_slider.hpp"

namespace color_widgets {

/**
 * \brief A slider for selecting a hue value
 */
class QCP_EXPORT HueSlider : public GradientSlider
{
    Q_OBJECT
    /**
     * \brief Saturation used in the rainbow gradient, as a [0-1] float
     */
    Q_PROPERTY(qreal colorSaturation READ colorSaturation WRITE setColorSaturation NOTIFY colorSaturationChanged)
    /**
     * \brief Value used in the rainbow gradient, as a [0-1] float
     */
    Q_PROPERTY(qreal colorValue READ colorValue WRITE setColorValue NOTIFY colorValueChanged)
    /**
     * \brief Alpha used in the rainbow gradient, as a [0-1] float
     */
    Q_PROPERTY(qreal colorAlpha READ colorAlpha WRITE setColorAlpha NOTIFY colorAlphaChanged)

    /**
     * \brief Color with corresponding color* components
     */
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)

    /**
     * \brief Normalized Hue, as indicated from the slider
     */
    Q_PROPERTY(qreal colorHue READ colorHue WRITE setColorHue NOTIFY colorHueChanged)


public:
    explicit HueSlider(QWidget *parent = nullptr);
    explicit HueSlider(Qt::Orientation orientation, QWidget *parent = nullptr);
    ~HueSlider();

    qreal colorSaturation() const;
    qreal colorValue() const;
    qreal colorAlpha() const;
    QColor color() const;
    qreal colorHue() const;

public Q_SLOTS:
    void setColorValue(qreal value);
    void setColorSaturation(qreal value);
    void setColorAlpha(qreal alpha);
    void setColorHue(qreal colorHue);
    /**
     * \brief Set Hue Saturation and ColorValue, ignoring alpha
     */
    void setColor(const QColor& color);
    /**
     * \brief Set Hue Saturation, ColorValue and Alpha
     */
    void setFullColor(const QColor& color);

Q_SIGNALS:
    void colorHueChanged(qreal colorHue);
    void colorChanged(QColor);
    void colorAlphaChanged(qreal v);
    void colorSaturationChanged(qreal v);
    void colorValueChanged(qreal v);

private:
    class Private;
    Private * const p;
};

} // namespace color_widgets

#endif // HUE_SLIDER_HPP

