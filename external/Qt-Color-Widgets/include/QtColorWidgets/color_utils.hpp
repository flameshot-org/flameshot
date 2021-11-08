/**
 * \file
 *
 * \author Mattia Basaglia
 *
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
#ifndef COLOR_UTILS_HPP
#define COLOR_UTILS_HPP

#include <QColor>
#include <QPoint>
#include <qmath.h>

#include "QtColorWidgets/colorwidgets_global.hpp"

namespace color_widgets {
namespace utils {


QCP_EXPORT inline qreal color_chromaF(const QColor& c)
{
    qreal max = qMax(c.redF(), qMax(c.greenF(), c.blueF()));
    qreal min = qMin(c.redF(), qMin(c.greenF(), c.blueF()));
    return max - min;
}

QCP_EXPORT inline qreal color_lumaF(const QColor& c)
{
    return 0.30 * c.redF() + 0.59 * c.greenF() + 0.11 * c.blueF();
}

QCP_EXPORT QColor color_from_lch(qreal hue, qreal chroma, qreal luma, qreal alpha = 1 );

QCP_EXPORT inline QColor rainbow_lch(qreal hue)
{
    return color_from_lch(hue,1,1);
}

QCP_EXPORT inline QColor rainbow_hsv(qreal hue)
{
    return QColor::fromHsvF(hue,1,1);
}

QCP_EXPORT inline qreal color_lightnessF(const QColor& c)
{
    return ( qMax(c.redF(),qMax(c.greenF(),c.blueF())) +
             qMin(c.redF(),qMin(c.greenF(),c.blueF())) ) / 2;
}

QCP_EXPORT inline qreal color_HSL_saturationF(const QColor& col)
{
    qreal c = color_chromaF(col);
    qreal l = color_lightnessF(col);
    if ( qFuzzyCompare(l+1,1) || qFuzzyCompare(l+1,2) )
        return 0;
    return c / (1-qAbs(2*l-1));
}


QCP_EXPORT QColor color_from_hsl(qreal hue, qreal sat, qreal lig, qreal alpha = 1 );

QCP_EXPORT QColor get_screen_color(const QPoint &global_pos);

} // namespace utils
} // namespace color_widgets

#endif // COLOR_UTILS_HPP
