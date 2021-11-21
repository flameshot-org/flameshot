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

#include "QtColorWidgets/color_wheel.hpp"
#include "QtColorWidgets/color_utils.hpp"

#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>

namespace color_widgets {

enum MouseStatus
{
    Nothing,
    DragCircle,
    DragSquare
};

class ColorWheel::Private
{
private:
    ColorWheel * const w;

public:
    qreal hue, sat, val;
    bool backgroundIsDark;
    unsigned int wheel_width;
    MouseStatus mouse_status;
    QPixmap hue_ring;
    QImage inner_selector;
    std::vector<uint32_t> inner_selector_buffer;
    ColorSpaceEnum color_space = ColorHSV;
    bool rotating_selector = true;
    ShapeEnum selector_shape = ShapeTriangle;
    QColor (*color_from)(qreal,qreal,qreal,qreal);
    QColor (*rainbow_from_hue)(qreal);
    int max_size = 128;

    Private(ColorWheel *widget)
        : w(widget), hue(0), sat(0), val(0),
        wheel_width(20), mouse_status(Nothing),
        color_from(&QColor::fromHsvF), rainbow_from_hue(&utils::rainbow_hsv)
    {
    }

    void setup()
    {
        qreal backgroundValue = w->palette().window().color().valueF();
        backgroundIsDark = backgroundValue < 0.5;
    }

    virtual ~Private(){}

    /// Calculate outer wheel radius from idget center
    qreal outer_radius() const
    {
        return qMin(w->geometry().width(), w->geometry().height())/2;
    }

    /// Calculate inner wheel radius from idget center
    qreal inner_radius() const
    {
        return outer_radius()-wheel_width;
    }

    /// Calculate the edge length of the inner square
    qreal square_size() const
    {
        return inner_radius()*qSqrt(2);
    }

    /// Calculate the height of the inner triangle
    qreal triangle_height() const
    {
        return inner_radius()*3/2;
    }

    /// Calculate the side of the inner triangle
    qreal triangle_side() const
    {
        return inner_radius()*qSqrt(3);
    }

    /// return line from center to given point
    QLineF line_to_point(const QPoint &p) const
    {
        return QLineF (w->geometry().width()/2, w->geometry().height()/2, p.x(), p.y());
    }

    /**
     * Ensures the internal image buffer is the correct size
     * and that the QImage is associated to it
     */
    void init_buffer(QSize size)
    {
        std::size_t linear_size = size.width() * size.height();
        if ( inner_selector_buffer.size() == linear_size )
            return;
        inner_selector_buffer.resize(linear_size);
        inner_selector = QImage(
            reinterpret_cast<uchar*>(inner_selector_buffer.data()),
            size.width(),
            size.height(),
            QImage::Format_RGB32
        );
    }

    void render_square()
    {
        int width = qMin<int>(square_size(), max_size);
        init_buffer(QSize(width, width));

        for ( int y = 0; y < width; ++y )
        {
            for ( int x = 0; x < width; ++x )
            {
                QRgb color = color_from(hue,double(x)/width,double(y)/width,1).rgb();
                inner_selector_buffer[width * y + x] = color;
            }
        }
    }

    /**
     * \brief renders the selector as a triangle
     * \note It's the same as a square with the edge with value=0 collapsed to a single point
     */
    void render_triangle()
    {
        QSizeF size = selector_size();
        if ( size.height() > max_size )
            size *= max_size / size.height();

        qreal ycenter = size.height()/2;

        QSize isize = size.toSize();
        init_buffer(isize);

        for (int x = 0; x < isize.width(); x++ )
        {
            qreal pval = x / size.height();
            qreal slice_h = size.height() * pval;
            for (int y = 0; y < isize.height(); y++ )
            {
                qreal ymin = ycenter-slice_h/2;
                qreal psat = qBound(0.0,(y-ymin)/slice_h,1.0);
                QRgb color = color_from(hue,psat,pval,1).rgb();
                inner_selector_buffer[isize.width() * y + x] = color;
            }
        }
    }

    /// Updates the inner image that displays the saturation-value selector
    void render_inner_selector()
    {
        if ( selector_shape == ShapeTriangle )
            render_triangle();
        else
            render_square();
    }

    /// Offset of the selector image
    QPointF selector_image_offset()
    {
        if ( selector_shape == ShapeTriangle )
                return QPointF(-inner_radius(),-triangle_side()/2);
        return QPointF(-square_size()/2,-square_size()/2);
    }

    /**
     * \brief Size of the selector when rendered to the screen
     */
    QSizeF selector_size()
    {
        if ( selector_shape == ShapeTriangle )
                return QSizeF(triangle_height(), triangle_side());
        return QSizeF(square_size(), square_size());
    }


    /// Rotation of the selector image
    qreal selector_image_angle()
    {
        if ( selector_shape == ShapeTriangle )
        {
            if ( rotating_selector )
                return -hue*360-60;
            return -150;
        }
        else
        {
            if ( rotating_selector )
                return -hue*360-45;
            else
                return 180;
        }
    }

    /// Updates the outer ring that displays the hue selector
    void render_ring()
    {
        hue_ring = QPixmap(outer_radius()*2,outer_radius()*2);
        hue_ring.fill(Qt::transparent);
        QPainter painter(&hue_ring);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setCompositionMode(QPainter::CompositionMode_Source);


        const int hue_stops = 24;
        QConicalGradient gradient_hue(0, 0, 0);
        if ( gradient_hue.stops().size() < hue_stops )
        {
            for ( double a = 0; a < 1.0; a+=1.0/(hue_stops-1) )
            {
                gradient_hue.setColorAt(a,rainbow_from_hue(a));
            }
            gradient_hue.setColorAt(1,rainbow_from_hue(0));
        }

        painter.translate(outer_radius(),outer_radius());

        painter.setPen(Qt::NoPen);
        painter.setBrush(QBrush(gradient_hue));
        painter.drawEllipse(QPointF(0,0),outer_radius(),outer_radius());

        painter.setBrush(Qt::transparent);//palette().background());
        painter.drawEllipse(QPointF(0,0),inner_radius(),inner_radius());
    }

    void set_color(const QColor& c)
    {
        float nice_hue = c.hsvHueF();
        if ( nice_hue < 0 )
            nice_hue = c.hslHueF();
        if ( nice_hue < 0 )
            nice_hue = hue;

        switch ( color_space )
        {
            case ColorHSV:
                hue = nice_hue;
                sat = c.hsvSaturationF();
                val = c.valueF();
                break;
            case ColorHSL:
                hue = nice_hue;
                sat = utils::color_HSL_saturationF(c);
                val = utils::color_lightnessF(c);
                break;
            case ColorLCH:
                hue = nice_hue;
                sat = utils::color_chromaF(c);
                val = utils::color_lumaF(c);
                break;
        }
    }

    void draw_ring_editor(double editor_hue, QPainter& painter, QColor color) {
        painter.setPen(QPen(color,3));
        painter.setBrush(Qt::NoBrush);
        QLineF ray(0, 0, outer_radius(), 0);
        ray.setAngle(editor_hue*360);
        QPointF h1 = ray.p2();
        ray.setLength(inner_radius());
        QPointF h2 = ray.p2();
        painter.drawLine(h1,h2);
    }

};

} //  namespace color_widgets
