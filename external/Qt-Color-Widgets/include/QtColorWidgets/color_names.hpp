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
#ifndef COLOR_WIDGETS_COLOR_NAMES_HPP
#define COLOR_WIDGETS_COLOR_NAMES_HPP

#include <QColor>
#include <QString>

#include <QtColorWidgets/colorwidgets_global.hpp>

namespace color_widgets {

/**
 * \brief Convert a string into a color
 *
 * Supported string formats:
 *  * Short hex strings #f00
 *  * Long hex strings  #ff0000
 *  * Color names       red
 *  * Function-like     rgb(255,0,0)
 *
 * Additional string formats supported only when \p alpha is true:
 *  * Long hex strings  #ff0000ff
 *  * Function like     rgba(255,0,0,255)
 */
QCP_EXPORT QColor colorFromString(const QString& string, bool alpha = true);

/**
 * \brief Convert a color into a string
 *
 * Format:
 *  * If the color has full alpha: #ff0000
 *  * If alpha is true and the color has non-full alpha: #ff000088
 */
QCP_EXPORT QString stringFromColor(const QColor& color, bool alpha = true);

} // namespace color_widgets
#endif // COLOR_WIDGETS_COLOR_NAMES_HPP
