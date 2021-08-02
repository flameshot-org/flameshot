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
#ifndef COLOR_WHEEL_HPP
#define COLOR_WHEEL_HPP

#include <QWidget>

#include "colorwidgets_global.hpp"


namespace color_widgets {

/**
 * \brief Display an analog widget that allows the selection of a HSV color
 *
 * It has an outer wheel to select the Hue and an intenal square to select
 * Saturation and Lightness.
 */
class QCP_EXPORT ColorWheel : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged DESIGNABLE true STORED false )
    Q_PROPERTY(qreal hue READ hue WRITE setHue DESIGNABLE false )
    Q_PROPERTY(qreal saturation READ saturation WRITE setSaturation DESIGNABLE false )
    Q_PROPERTY(qreal value READ value WRITE setValue DESIGNABLE false )
    Q_PROPERTY(unsigned wheelWidth READ wheelWidth WRITE setWheelWidth NOTIFY wheelWidthChanged DESIGNABLE true )
    Q_PROPERTY(ShapeEnum selectorShape READ selectorShape WRITE setSelectorShape NOTIFY selectorShapeChanged DESIGNABLE true )
    Q_PROPERTY(bool rotatingSelector READ rotatingSelector WRITE setRotatingSelector NOTIFY rotatingSelectorChanged DESIGNABLE true )
    Q_PROPERTY(ColorSpaceEnum colorSpace READ colorSpace WRITE setColorSpace NOTIFY colorSpaceChanged DESIGNABLE true )

public:
    enum ShapeEnum
    {
        ShapeTriangle,  ///< A triangle
        ShapeSquare,    ///< A square
    };

    enum AngleEnum
    {
        AngleFixed,     ///< The inner part doesn't rotate
        AngleRotating,  ///< The inner part follows the hue selector
    };

    enum ColorSpaceEnum
    {
        ColorHSV,       ///< Use the HSV color space
        ColorHSL,       ///< Use the HSL color space
        ColorLCH,       ///< Use Luma Chroma Hue (Y_601')
    };

    Q_ENUM(ShapeEnum);
    Q_ENUM(AngleEnum);
    Q_ENUM(ColorSpaceEnum);

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

    /// Shape of the internal selector
    ShapeEnum selectorShape() const;

    /// Whether the internal selector should rotare in accordance with the hue
    bool rotatingSelector() const;

    /// Color space used to preview/edit the color
    ColorSpaceEnum colorSpace() const;

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

    /// Sets the shape of the internal selector
    void setSelectorShape(ShapeEnum shape);

    /// Sets whether the internal selector should rotare in accordance with the hue
    void setRotatingSelector(bool rotating);

    /// Sets the color space used to preview/edit the color
    void setColorSpace(ColorSpaceEnum space);

Q_SIGNALS:
    /**
     * Emitted when the user selects a color or setColor is called
     */
    void colorChanged(QColor);

    /**
     * Emitted when the user selects a color
     */
    void colorSelected(QColor);

    void wheelWidthChanged(unsigned);

    void selectorShapeChanged(ShapeEnum shape);

    void rotatingSelectorChanged(bool rotating);

    void colorSpaceChanged(ColorSpaceEnum space);

    /**
     * Emitted when the user releases from dragging
     */
    void editingFinished();

protected:
    void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *) Q_DECL_OVERRIDE;
    void dragEnterEvent(QDragEnterEvent* event) Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent* event) Q_DECL_OVERRIDE;

protected:
    class Private;
    ColorWheel(QWidget *parent, Private* data);
    Private* data() const { return p; }

private:
    Private * const p;

};

} // namespace color_widgets

#endif // COLOR_WHEEL_HPP
