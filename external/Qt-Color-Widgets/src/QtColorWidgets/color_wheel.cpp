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
#include "QtColorWidgets/color_wheel.hpp"
#include "QtColorWidgets/color_wheel_private.hpp"

#include <cmath>
#include <QLineF>
#include <QDragEnterEvent>
#include <QMimeData>

namespace color_widgets {


static const double selector_radius = 6;


ColorWheel::ColorWheel(QWidget *parent, Private* data) :
    QWidget(parent), p(data)
{
    p->setup();
    setAcceptDrops(true);
}


ColorWheel::ColorWheel(QWidget *parent) :
    ColorWheel(parent, new Private(this))
{

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
    if ( p->color_space == ColorLCH && p->sat > 0.01 )
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
    Q_EMIT wheelWidthChanged(w);
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
    p->draw_ring_editor(p->hue, painter, Qt::black);

    // lum-sat square
    if(p->inner_selector.isNull())
        p->render_inner_selector();

    painter.rotate(p->selector_image_angle());
    painter.translate(p->selector_image_offset());

    QPointF selector_position;
    if ( p->selector_shape == ShapeSquare )
    {
        qreal side = p->square_size();
        selector_position = QPointF(p->sat*side, p->val*side);
    }
    else if ( p->selector_shape == ShapeTriangle )
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
    // in order to improve to contrast
    if (p->backgroundIsDark)
    {
        bool isWhite = (p->val < 0.65 || p->sat > 0.43);
        painter.setPen(QPen(isWhite ? Qt::white : Qt::black, 3));
    }
    else
    {
        painter.setPen(QPen(p->val > 0.5 ? Qt::black : Qt::white, 3));
    }
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(selector_position, selector_radius, selector_radius);

}

void ColorWheel::mouseMoveEvent(QMouseEvent *ev)
{
    if (p->mouse_status == DragCircle )
    {
        auto hue = p->line_to_point(ev->pos()).angle()/360.0;
        p->hue = hue;
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

        if ( p->selector_shape == ShapeSquare )
        {
            p->sat = qBound(0.0, center_mouse_ln.x2()/p->square_size(), 1.0);
            p->val = qBound(0.0, center_mouse_ln.y2()/p->square_size(), 1.0);
        }
        else if ( p->selector_shape == ShapeTriangle )
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
    if ( ev->button() == Qt::LeftButton )
        Q_EMIT editingFinished();
}

void ColorWheel::resizeEvent(QResizeEvent *)
{
    static bool skipFirst = true;
    // Skip the first time in order to prevent QPainter warning messages
    if (!skipFirst)
        p->render_ring();
    skipFirst = false;
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

color_widgets::ColorWheel::ColorSpaceEnum ColorWheel::colorSpace() const
{
    return p->color_space;
}

bool ColorWheel::rotatingSelector() const
{
    return p->rotating_selector;
}

color_widgets::ColorWheel::ShapeEnum ColorWheel::selectorShape() const
{
    return p->selector_shape;
}


void ColorWheel::setColorSpace(color_widgets::ColorWheel::ColorSpaceEnum space)
{
    if ( p->color_space != space )
    {
        p->color_space = space;

        QColor old_col = color();

        switch ( space )
        {
            case ColorHSL:
                p->hue = old_col.hueF();
                p->sat = utils::color_HSL_saturationF(old_col);
                p->val = utils::color_lightnessF(old_col);
                p->color_from = &utils::color_from_hsl;
                p->rainbow_from_hue = &utils::rainbow_hsv;
                break;
            case ColorHSV:
                p->hue = old_col.hsvHueF();
                p->sat = old_col.hsvSaturationF();
                p->val = old_col.valueF();
                p->color_from = &QColor::fromHsvF;
                p->rainbow_from_hue = &utils::rainbow_hsv;
                break;
            case ColorLCH:
                p->hue = old_col.hueF();
                p->sat = utils::color_chromaF(old_col);
                p->val = utils::color_lumaF(old_col);
                p->color_from = &utils::color_from_lch;
                p->rainbow_from_hue = &utils::rainbow_lch;
                break;
        }

        p->render_ring();
        p->render_inner_selector();
        update();
        Q_EMIT colorSpaceChanged(space);
    }
}

void ColorWheel::setRotatingSelector(bool rotating)
{
    p->rotating_selector = rotating;
    update();
    Q_EMIT rotatingSelectorChanged(rotating);
}

void ColorWheel::setSelectorShape(color_widgets::ColorWheel::ShapeEnum shape)
{
    if ( shape != p->selector_shape )
    {
        p->selector_shape = shape;
        update();
        p->render_inner_selector();
        Q_EMIT selectorShapeChanged(shape);
    }
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
