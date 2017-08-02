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
#include "color_wheel.hpp"

#include <cmath>
#include <QMouseEvent>
#include <QPainter>
#include <QLineF>
#include <QDragEnterEvent>
#include <QMimeData>
#include "color_utils.hpp"

namespace color_widgets {

enum MouseStatus
{
    Nothing,
    DragCircle,
    DragSquare
};

static const ColorWheel::DisplayFlags hard_default_flags = ColorWheel::SHAPE_TRIANGLE|ColorWheel::ANGLE_ROTATING|ColorWheel::COLOR_HSV;
static ColorWheel::DisplayFlags default_flags = hard_default_flags;
static const double selector_radius = 6;

class ColorWheel::Private
{
private:
    ColorWheel * const w;

public:
    qreal hue, sat, val;
    qreal bgBrightness;
    unsigned int wheel_width;
    MouseStatus mouse_status;
    QPixmap hue_ring;
    QImage inner_selector;
    DisplayFlags display_flags;
    QColor (*color_from)(qreal,qreal,qreal,qreal);
    QColor (*rainbow_from_hue)(qreal);
    int max_size = 128;

    Private(ColorWheel *widget)
        : w(widget), hue(0), sat(0), val(0),
        wheel_width(20), mouse_status(Nothing),
        display_flags(FLAGS_DEFAULT),
        color_from(&QColor::fromHsvF), rainbow_from_hue(&detail::rainbow_hsv)
    {
        QColor bgColor = widget->palette().background().color();
        bgBrightness = color_widgets::detail::color_lumaF(bgColor);
    }

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

    void render_square()
    {
        int width = qMin<int>(square_size(), max_size);
        QSize size(width, width);
        inner_selector = QImage(size, QImage::Format_RGB32);

        for ( int y = 0; y < width; ++y )
        {
            for ( int x = 0; x < width; ++x )
            {
                inner_selector.setPixel( x, y,
                        color_from(hue,double(x)/width,double(y)/width,1).rgb());
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
        inner_selector = QImage(size.toSize(), QImage::Format_RGB32);

        for (int x = 0; x < inner_selector.width(); x++ )
        {
            qreal pval = x / size.height();
            qreal slice_h = size.height() * pval;
            for (int y = 0; y < inner_selector.height(); y++ )
            {
                qreal ymin = ycenter-slice_h/2;
                qreal psat = qBound(0.0,(y-ymin)/slice_h,1.0);

                inner_selector.setPixel(x,y,color_from(hue,psat,pval,1).rgb());
            }
        }
    }

    /// Updates the inner image that displays the saturation-value selector
    void render_inner_selector()
    {
        if ( display_flags & ColorWheel::SHAPE_TRIANGLE )
            render_triangle();
        else
            render_square();
    }

    /// Offset of the selector image
    QPointF selector_image_offset()
    {
        if ( display_flags & SHAPE_TRIANGLE )
                return QPointF(-inner_radius(),-triangle_side()/2);
        return QPointF(-square_size()/2,-square_size()/2);
    }

    /**
     * \brief Size of the selector when rendered to the screen
     */
    QSizeF selector_size()
    {
        if ( display_flags & SHAPE_TRIANGLE )
                return QSizeF(triangle_height(), triangle_side());
        return QSizeF(square_size(), square_size());
    }


    /// Rotation of the selector image
    qreal selector_image_angle()
    {
        if ( display_flags & SHAPE_TRIANGLE )
        {
            if ( display_flags & ANGLE_ROTATING )
                return -hue*360-60;
            return -150;
        }
        else
        {
            if ( display_flags & ANGLE_ROTATING )
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
        if ( display_flags & ColorWheel::COLOR_HSV )
        {
            hue = qMax(0.0, c.hsvHueF());
            sat = c.hsvSaturationF();
            val = c.valueF();
        }
        else if ( display_flags & ColorWheel::COLOR_HSL )
        {
            hue = qMax(0.0, c.hueF());
            sat = detail::color_HSL_saturationF(c);
            val = detail::color_lightnessF(c);
        }
        else if ( display_flags & ColorWheel::COLOR_LCH )
        {
            hue = qMax(0.0, c.hsvHueF());
            sat = detail::color_chromaF(c);
            val = detail::color_lumaF(c);
        }
    }
};

ColorWheel::ColorWheel(QWidget *parent) :
    QWidget(parent), p(new Private(this))
{
    setDisplayFlags(FLAGS_DEFAULT);
    setAcceptDrops(true);
}

ColorWheel::~ColorWheel()
{
    delete p;
}

QColor ColorWheel::color() const
{
    return p->color_from(p->hue, p->sat, p->val, 1);
}

QSize ColorWheel::sizeHint() const
{
    return QSize(p->wheel_width*5, p->wheel_width*5);
}

qreal ColorWheel::hue() const
{
    if ( (p->display_flags & COLOR_LCH) && p->sat > 0.01 )
        return color().hueF();
    return p->hue;
}

qreal ColorWheel::saturation() const
{
    return color().hsvSaturationF();
}

qreal ColorWheel::value() const
{
    return color().valueF();
}

unsigned int ColorWheel::wheelWidth() const
{
    return p->wheel_width;
}

void ColorWheel::setWheelWidth(unsigned int w)
{
    p->wheel_width = w;
    p->render_inner_selector();
    update();
}

void ColorWheel::paintEvent(QPaintEvent * )
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.translate(geometry().width()/2,geometry().height()/2);

    // hue wheel
    if(p->hue_ring.isNull())
        p->render_ring();

    painter.drawPixmap(-p->outer_radius(), -p->outer_radius(), p->hue_ring);

    // hue selector
    QColor penColor = p->bgBrightness < 0.6 ? Qt::white : Qt::black;
    painter.setPen(QPen(penColor,3));
    painter.setBrush(Qt::NoBrush);
    QLineF ray(0, 0, p->outer_radius(), 0);
    ray.setAngle(p->hue*360);
    QPointF h1 = ray.p2();
    ray.setLength(p->inner_radius());
    QPointF h2 = ray.p2();
    painter.drawLine(h1,h2);

    // lum-sat square
    if(p->inner_selector.isNull())
        p->render_inner_selector();

    painter.rotate(p->selector_image_angle());
    painter.translate(p->selector_image_offset());

    QPointF selector_position;
    if ( p->display_flags & SHAPE_SQUARE )
    {
        qreal side = p->square_size();
        selector_position = QPointF(p->sat*side, p->val*side);
    }
    else if ( p->display_flags & SHAPE_TRIANGLE )
    {
        qreal side = p->triangle_side();
        qreal height = p->triangle_height();
        qreal slice_h = side * p->val;
        qreal ymin = side/2-slice_h/2;

        selector_position = QPointF(p->val*height, ymin + p->sat*slice_h);
        QPolygonF triangle;
        triangle.append(QPointF(0,side/2));
        triangle.append(QPointF(height,0));
        triangle.append(QPointF(height,side));
        QPainterPath clip;
        clip.addPolygon(triangle);
        painter.setClipPath(clip);
    }

    painter.drawImage(QRectF(QPointF(0, 0), p->selector_size()), p->inner_selector);
    painter.setClipping(false);

    // lum-sat selector
    // we define the color of the selecto based on the background color of the widget
    // in order to improve the contrast
    qreal colorBrightness = color_widgets::detail::color_lumaF(color());
    if (p->bgBrightness < 0.6) // dark theme
    {
        bool isWhite = (colorBrightness < 0.7);
        painter.setPen(QPen(isWhite ? Qt::white : Qt::black, 3));
    }
    else // light theme
    {
        bool isWhite = (colorBrightness < 0.4 && p->val < 0.3);
        painter.setPen(QPen(isWhite ? Qt::white : Qt::black, 3));
    }
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(selector_position, selector_radius, selector_radius);

}

void ColorWheel::mouseMoveEvent(QMouseEvent *ev)
{
    if (p->mouse_status == DragCircle )
    {
        p->hue = p->line_to_point(ev->pos()).angle()/360.0;
        p->render_inner_selector();

        Q_EMIT colorSelected(color());
        Q_EMIT colorChanged(color());
        update();
    }
    else if(p->mouse_status == DragSquare)
    {
        QLineF glob_mouse_ln = p->line_to_point(ev->pos());
        QLineF center_mouse_ln ( QPointF(0,0),
                                 glob_mouse_ln.p2() - glob_mouse_ln.p1() );

        center_mouse_ln.setAngle(center_mouse_ln.angle()+p->selector_image_angle());
        center_mouse_ln.setP2(center_mouse_ln.p2()-p->selector_image_offset());

        if ( p->display_flags & SHAPE_SQUARE )
        {
            p->sat = qBound(0.0, center_mouse_ln.x2()/p->square_size(), 1.0);
            p->val = qBound(0.0, center_mouse_ln.y2()/p->square_size(), 1.0);
        }
        else if ( p->display_flags & SHAPE_TRIANGLE )
        {
            QPointF pt = center_mouse_ln.p2();

            qreal side = p->triangle_side();
            p->val = qBound(0.0, pt.x() / p->triangle_height(), 1.0);
            qreal slice_h = side * p->val;

            qreal ycenter = side/2;
            qreal ymin = ycenter-slice_h/2;

            if ( slice_h > 0 )
                p->sat = qBound(0.0, (pt.y()-ymin)/slice_h, 1.0);
        }

        Q_EMIT colorSelected(color());
        Q_EMIT colorChanged(color());
        update();
    }
}

void ColorWheel::mousePressEvent(QMouseEvent *ev)
{
    if ( ev->buttons() & Qt::LeftButton )
    {
        setFocus();
        QLineF ray = p->line_to_point(ev->pos());
        if ( ray.length() <= p->inner_radius() )
            p->mouse_status = DragSquare;
        else if ( ray.length() <= p->outer_radius() )
            p->mouse_status = DragCircle;

        // Update the color
        mouseMoveEvent(ev);
    }
}

void ColorWheel::mouseReleaseEvent(QMouseEvent *ev)
{
    mouseMoveEvent(ev);
    p->mouse_status = Nothing;
    Q_EMIT mouseReleaseOnColor(color());

}

void ColorWheel::resizeEvent(QResizeEvent *)
{
    p->render_ring();
    p->render_inner_selector();
}

void ColorWheel::setColor(QColor c)
{
    qreal oldh = p->hue;
    p->set_color(c);
    if (!qFuzzyCompare(oldh+1, p->hue+1))
        p->render_inner_selector();
    update();
    Q_EMIT colorChanged(c);
}

void ColorWheel::setHue(qreal h)
{
    p->hue = qBound(0.0, h, 1.0);
    p->render_inner_selector();
    update();
}

void ColorWheel::setSaturation(qreal s)
{
    p->sat = qBound(0.0, s, 1.0);
    update();
}

void ColorWheel::setValue(qreal v)
{
    p->val = qBound(0.0, v, 1.0);
    update();
}


void ColorWheel::setDisplayFlags(DisplayFlags flags)
{
    if ( ! (flags & COLOR_FLAGS) )
        flags |= default_flags & COLOR_FLAGS;
    if ( ! (flags & ANGLE_FLAGS) )
        flags |= default_flags & ANGLE_FLAGS;
    if ( ! (flags & SHAPE_FLAGS) )
        flags |= default_flags & SHAPE_FLAGS;

    if ( (flags & COLOR_FLAGS) != (p->display_flags & COLOR_FLAGS) )
    {
        QColor old_col = color();
        if ( flags & ColorWheel::COLOR_HSL )
        {
            p->hue = old_col.hueF();
            p->sat = detail::color_HSL_saturationF(old_col);
            p->val = detail::color_lightnessF(old_col);
            p->color_from = &detail::color_from_hsl;
            p->rainbow_from_hue = &detail::rainbow_hsv;
        }
        else if ( flags & ColorWheel::COLOR_LCH )
        {
            p->hue = old_col.hueF();
            p->sat = detail::color_chromaF(old_col);
            p->val = detail::color_lumaF(old_col);
            p->color_from = &detail::color_from_lch;
            p->rainbow_from_hue = &detail::rainbow_lch;
        }
        else
        {
            p->hue = old_col.hsvHueF();
            p->sat = old_col.hsvSaturationF();
            p->val = old_col.valueF();
            p->color_from = &QColor::fromHsvF;
            p->rainbow_from_hue = &detail::rainbow_hsv;
        }
        p->render_ring();
    }

    p->display_flags = flags;
    p->render_inner_selector();
    update();
    Q_EMIT displayFlagsChanged(flags);
}

ColorWheel::DisplayFlags ColorWheel::displayFlags(DisplayFlags mask) const
{
    return p->display_flags & mask;
}

void ColorWheel::setDefaultDisplayFlags(DisplayFlags flags)
{
    if ( !(flags & COLOR_FLAGS) )
        flags |= hard_default_flags & COLOR_FLAGS;
    if ( !(flags & ANGLE_FLAGS) )
        flags |= hard_default_flags & ANGLE_FLAGS;
    if ( !(flags & SHAPE_FLAGS) )
        flags |= hard_default_flags & SHAPE_FLAGS;
    default_flags = flags;
}

ColorWheel::DisplayFlags ColorWheel::defaultDisplayFlags(DisplayFlags mask)
{
    return default_flags & mask;
}

void ColorWheel::setDisplayFlag(DisplayFlags flag, DisplayFlags mask)
{
    setDisplayFlags((p->display_flags&~mask)|flag);
}

void ColorWheel::dragEnterEvent(QDragEnterEvent* event)
{
    if ( event->mimeData()->hasColor() ||
         ( event->mimeData()->hasText() && QColor(event->mimeData()->text()).isValid() ) )
        event->acceptProposedAction();
}

void ColorWheel::dropEvent(QDropEvent* event)
{
    if ( event->mimeData()->hasColor() )
    {
        setColor(event->mimeData()->colorData().value<QColor>());
        event->accept();
    }
    else if ( event->mimeData()->hasText() )
    {
        QColor col(event->mimeData()->text());
        if ( col.isValid() )
        {
            setColor(col);
            event->accept();
        }
    }
}

} //  namespace color_widgets
