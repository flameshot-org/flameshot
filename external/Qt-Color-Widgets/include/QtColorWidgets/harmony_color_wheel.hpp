/**
 * \file
 *
 * \author Mattia Basaglia
 *
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
#ifndef HARMONY_COLOR_WHEEL_HPP
#define HARMONY_COLOR_WHEEL_HPP


#include "color_wheel.hpp"

namespace color_widgets {

/**
 * \brief ColorWheel with color harmonies
 */
class QCP_EXPORT HarmonyColorWheel : public ColorWheel
{
    Q_OBJECT

public:
    explicit HarmonyColorWheel(QWidget *parent = 0);
    ~HarmonyColorWheel();

    /// Get all harmony colors (including main)
    QList<QColor> harmonyColors() const;

    /// Get number of harmony colors (including main)
    unsigned int harmonyCount() const;

    /// Clear harmony color scheme
    void clearHarmonies();

    /**
     * @brief Add harmony color
     * @param hue_diff     Initial hue difference (in [0-1) range)
     * @param editable     Whether this harmony should be editable
     * @returns Index of newly added harmony
     */
    unsigned addHarmony(double hue_diff, bool editable);

    /**
     * @brief Add symmetric harmony color
     * @param relative_to  Index of other harmony that should be symmetric relative to main hue
     * @returns Index of newly added harmony
     * Editability is inherited from symmetric editor
     */
    unsigned addSymmetricHarmony(unsigned relative_to);

    /**
     * @brief Add opposite harmony color
     * @param relative_to  Index of other harmony that should be opposite to this
     * @returns Index of newly added harmony
     * Editability is inherited from opposite editor
     */
    unsigned addOppositeHarmony(unsigned relative_to);

Q_SIGNALS:
    /**
     * Emitted when harmony settings or harmony colors are changed (including due to main hue change)
     */
    void harmonyChanged();

protected:
    void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *) Q_DECL_OVERRIDE;

private:
    class Private;
    Private * p;
};

} // namespace color_widgets

#endif // COLOR_WHEEL_HPP

