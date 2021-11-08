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
#ifndef COLOR_WIDGETS_COLOR_2D_SLIDER_HPP
#define COLOR_WIDGETS_COLOR_2D_SLIDER_HPP

#include "colorwidgets_global.hpp"
#include <QWidget>

namespace color_widgets {

/**
 * \brief A 2D slider that edits 2 color components
 */
class QCP_EXPORT Color2DSlider : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged DESIGNABLE true STORED false )
    Q_PROPERTY(qreal hue READ hue WRITE setHue DESIGNABLE false )
    Q_PROPERTY(qreal saturation READ saturation WRITE setSaturation DESIGNABLE false )
    Q_PROPERTY(qreal value READ value WRITE setValue DESIGNABLE false )
    /**
     * \brief Which color component is used on the x axis
     */
    Q_PROPERTY(Component componentX READ componentX WRITE setComponentX NOTIFY componentXChanged)
    /**
     * \brief Which color component is used on the y axis
     */
    Q_PROPERTY(Component componentY READ componentY WRITE setComponentY NOTIFY componentYChanged)


public:
    enum Component {
        Hue, Saturation, Value
    };
    Q_ENUMS(Component)

    explicit Color2DSlider(QWidget *parent = nullptr);
    ~Color2DSlider();

    /// Get current color
    QColor color() const;

    QSize sizeHint() const Q_DECL_OVERRIDE;

    /// Get current hue in the range [0-1]
    qreal hue() const;

    /// Get current saturation in the range [0-1]
    qreal saturation() const;

    /// Get current value in the range [0-1]
    qreal value() const;

    Component componentX() const;
    Component componentY() const;

public Q_SLOTS:

    /// Set current color
    void setColor(const QColor& c);

    /**
     * @param h Hue [0-1]
     */
    void setHue(qreal h);

    /**
     * @param s Saturation [0-1]
     */
    void setSaturation(qreal s);

    /**
     * @param v Value [0-1]
     */
    void setValue(qreal v);

    void setComponentX(Component componentX);
    void setComponentY(Component componentY);

Q_SIGNALS:
    /**
     * Emitted when the user selects a color or setColor is called
     */
    void colorChanged(QColor);

    /**
     * Emitted when the user selects a color
     */
    void colorSelected(QColor);

    void componentXChanged(Component componentX);
    void componentYChanged(Component componentY);

protected:
    void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

private:
    class Private;
    Private * const p;
};

} // namespace color_widgets

#endif // COLOR_WIDGETS_COLOR_2D_SLIDER_HPP
