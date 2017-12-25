/**
 * \file
 *
 * \author Mattia Basaglia
 *
 * \copyright Copyright (C) 2013-2017 Mattia Basaglia
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
#ifndef COLOR_WHEEL_HPP
#define COLOR_WHEEL_HPP

#include <QWidget>

namespace color_widgets {

/**
 * \brief Display an analog widget that allows the selection of a HSV color
 *
 * It has an outer wheel to select the Hue and an intenal square to select
 * Saturation and Lightness.
 */
class ColorWheel : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged DESIGNABLE true STORED false )
    Q_PROPERTY(qreal hue READ hue WRITE setHue DESIGNABLE false )
    Q_PROPERTY(qreal saturation READ saturation WRITE setSaturation DESIGNABLE false )
    Q_PROPERTY(qreal value READ value WRITE setValue DESIGNABLE false )
    Q_PROPERTY(unsigned wheelWidth READ wheelWidth WRITE setWheelWidth DESIGNABLE true )
    Q_PROPERTY(DisplayFlags displayFlags READ displayFlags WRITE setDisplayFlags NOTIFY displayFlagsChanged DESIGNABLE true )

public:
    enum DisplayEnum
    {
        SHAPE_DEFAULT  = 0x000, ///< Use the default shape
        SHAPE_TRIANGLE = 0x001, ///< A triangle
        SHAPE_SQUARE   = 0x002, ///< A square
        SHAPE_FLAGS    = 0x00f, ///< Mask for the shape flags

        ANGLE_DEFAULT  = 0x000, ///< Use the default rotation style
        ANGLE_FIXED    = 0x010, ///< The inner part doesn't rotate
        ANGLE_ROTATING = 0x020, ///< The inner part follows the hue selector
        ANGLE_FLAGS    = 0x0f0, ///< Mask for the angle flags

        COLOR_DEFAULT  = 0x000, ///< Use the default colorspace
        COLOR_HSV      = 0x100, ///< Use the HSV color space
        COLOR_HSL      = 0x200, ///< Use the HSL color space
        COLOR_LCH      = 0x400, ///< Use Luma Chroma Hue (Y_601')
        COLOR_FLAGS    = 0xf00, ///< Mask for the color space flags

        FLAGS_DEFAULT  = 0x000, ///< Use all defaults
        FLAGS_ALL      = 0xfff  ///< Mask matching all flags
    };
    Q_DECLARE_FLAGS(DisplayFlags, DisplayEnum)
    Q_FLAGS(DisplayFlags)

    explicit ColorWheel(QWidget *parent = 0);
    ~ColorWheel();

    /// Get current color
    QColor color() const;

    virtual QSize sizeHint() const Q_DECL_OVERRIDE;

    /// Get current hue in the range [0-1]
    qreal hue() const;

    /// Get current saturation in the range [0-1]
    qreal saturation() const;

    /// Get current value in the range [0-1]
    qreal value() const;

    /// Get the width in pixels of the outer wheel
    unsigned int wheelWidth() const;

    /// Set the width in pixels of the outer wheel
    void setWheelWidth(unsigned int w);

    /// Get display flags
    DisplayFlags displayFlags(DisplayFlags mask = FLAGS_ALL) const;

    /// Set the default display flags
    static void setDefaultDisplayFlags(DisplayFlags flags);

    /// Get default display flags
    static DisplayFlags defaultDisplayFlags(DisplayFlags mask = FLAGS_ALL);

    /**
     * @brief Set a specific display flag
     * @param flag  Flag replacing the mask
     * @param mask  Mask to be cleared
     */
    void setDisplayFlag(DisplayFlags flag, DisplayFlags mask);

public Q_SLOTS:

    /// Set current color
    void setColor(QColor c);

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

    /**
     * @brief Set the display flags
     * @param flags which will replace the current ones
     */
    void setDisplayFlags(ColorWheel::DisplayFlags flags);

Q_SIGNALS:
    /**
     * Emitted when the user selects a color or setColor is called
     */
    void colorChanged(QColor);

    /**
     * Emitted when the user selects a color
     */
    void colorSelected(QColor);

    void displayFlagsChanged(ColorWheel::DisplayFlags flags);

    void mouseReleaseOnColor(QColor);

protected:
    void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *) Q_DECL_OVERRIDE;
    void dragEnterEvent(QDragEnterEvent* event) Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent* event) Q_DECL_OVERRIDE;

private:
    class Private;
    Private * const p;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ColorWheel::DisplayFlags)

} // namespace color_widgets

#endif // COLOR_WHEEL_HPP
