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
#ifndef BOUND_COLOR_SELECTOR_HPP
#define BOUND_COLOR_SELECTOR_HPP

#include "color_selector.hpp"

namespace color_widgets {
/**
 * \brief A color selector bound to a color reference
 * \todo Maybe this can be removed
 */
class QCP_EXPORT BoundColorSelector : public ColorSelector
{
    Q_OBJECT
private:
    QColor* ref;
public:
    explicit BoundColorSelector(QColor* reference, QWidget *parent = 0);

private Q_SLOTS:
    void update_reference(QColor);
};
} // namespace color_widgets
#endif // BOUND_COLOR_SELECTOR_HPP
