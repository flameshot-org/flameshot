/**
 * \file gradient_slider.hpp
 *
 * \author Mattia Basaglia
 *
 * \copyright Copyright (C) 2013-2020 Mattia Basaglia
 * \copyright Copyright (C) 2014 Calle Laakkonen
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
#ifndef GRADIENT_SLIDER_HPP
#define GRADIENT_SLIDER_HPP

#include "colorwidgets_global.hpp"

#include <QSlider>
#include <QGradient>

namespace color_widgets {

/**
 * \brief A slider that moves on top of a gradient
 */
class QCP_EXPORT GradientSlider : public QSlider
{
    Q_OBJECT
    Q_PROPERTY(QBrush background READ background WRITE setBackground NOTIFY backgroundChanged)
    Q_PROPERTY(QGradientStops colors READ colors WRITE setColors DESIGNABLE false)
    Q_PROPERTY(QColor firstColor READ firstColor WRITE setFirstColor STORED false)
    Q_PROPERTY(QColor lastColor READ lastColor WRITE setLastColor STORED false)
    Q_PROPERTY(QLinearGradient gradient READ gradient WRITE setGradient)

public:
    explicit GradientSlider(QWidget *parent = 0);
    explicit GradientSlider(Qt::Orientation orientation, QWidget *parent = 0);
    ~GradientSlider();

    /// Get the background, it's visible for transparent gradient stops
    QBrush background() const;
    /// Set the background, it's visible for transparent gradient stops
    void setBackground(const QBrush &bg);

    /// Get the colors that make up the gradient
    QGradientStops colors() const;
    /// Set the colors that make up the gradient
    void setColors(const QGradientStops &colors);

    /// Get the gradient
    QLinearGradient gradient() const;
    /// Set the gradient
    void setGradient(const QLinearGradient &gradient);

    /**
     * Overload: create an evenly distributed gradient of the given colors
     */
    void setColors(const QVector<QColor> &colors);

    /**
     * \brief Set the first color of the gradient
     *
     * If the gradient is currently empty it will create a stop with the given color
     */
    void setFirstColor(const QColor &c);

    /**
     * \brief Set the last color of the gradient
     *
     * If the gradient is has less than two colors,
     * it will create a stop with the given color
     */
    void setLastColor(const QColor &c);

    /**
     * \brief Get the first color
     *
     * \returns QColor() con empty gradient
     */
    QColor firstColor() const;

    /**
     * \brief Get the last color
     *
     * \returns QColor() con empty gradient
     */
    QColor lastColor() const;

Q_SIGNALS:
    void backgroundChanged(const QBrush&);
    
protected:
    void paintEvent(QPaintEvent *ev) override;

    void mousePressEvent(QMouseEvent *ev) override;
    void mouseMoveEvent(QMouseEvent *ev) override;
    void mouseReleaseEvent(QMouseEvent *ev) override;

private:
    class Private;
    Private * const p;
};

} // namespace color_widgets

#endif // GRADIENT_SLIDER_HPP
