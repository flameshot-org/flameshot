/**
 * \file gradient_editor.cpp
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
#include "QtColorWidgets/gradient_editor.hpp"

#include <QPainter>
#include <QStyleOptionSlider>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QApplication>
#include <QDrag>
#include <QMimeData>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QMenu>

#include "QtColorWidgets/gradient_helper.hpp"
#include "QtColorWidgets/color_dialog.hpp"

namespace color_widgets {

class GradientEditor::Private
{
public:
    QGradientStops stops;
    QBrush back;
    Qt::Orientation orientation;
    int highlighted = -1;
    QLinearGradient gradient;
    int selected = -1;
    int drop_index = -1;
    QColor drop_color;
    qreal drop_pos = 0;
    ColorDialog color_dialog;
    int dialog_selected = -1;

    Private() :
        back(Qt::darkGray, Qt::DiagCrossPattern)
    {
        back.setTexture(QPixmap(QStringLiteral(":/color_widgets/alphaback.png")));
        gradient.setCoordinateMode(QGradient::StretchToDeviceMode);
        gradient.setSpread(QGradient::RepeatSpread);
    }

    void refresh_gradient()
    {
        gradient.setStops(stops);
    }

    qreal paint_pos(const QGradientStop& stop, const GradientEditor* owner)
    {
        return 2.5 + stop.first * (owner->geometry().width() - 5);
    }

    int closest(const QPoint& p, GradientEditor* owner)
    {
        if ( stops.empty() )
            return -1;
        if ( stops.size() == 1 || owner->geometry().width() <= 5 )
            return 0;
        qreal pos = move_pos(p, owner);

        int i = 1;
        for ( ; i < stops.size()-1; i++ )
            if ( stops[i].first >= pos )
                break;

        if ( stops[i].first - pos < pos - stops[i-1].first )
            return i;
        return i-1;
    }

    qreal move_pos(const QPoint& p, GradientEditor* owner)
    {
        int width;
        qreal x;
        if ( orientation == Qt::Horizontal )
        {
            width = owner->geometry().width();
            x = p.x();
        }
        else
        {
            width = owner->geometry().height();
            x = p.y();
        }
        return (width > 5) ? qMax(qMin((x - 2.5) / (width - 5), 1.0), 0.0) : 0;
    }

    void drop_event(QDropEvent* event, GradientEditor* owner)
    {
        drop_index = closest(event->pos(), owner);
        drop_pos = move_pos(event->pos(), owner);
        if ( drop_index == -1 )
            drop_index = stops.size();

        // Gather up the color
        if ( event->mimeData()->hasColor() )
            drop_color = event->mimeData()->colorData().value<QColor>();
        else if ( event->mimeData()->hasText() )
            drop_color = QColor(event->mimeData()->text());

        owner->update();
    }

    void clear_drop(GradientEditor* owner)
    {
        drop_index = -1;
        drop_color = QColor();
        owner->update();
    }

    void add_stop_data(int& index, qreal& pos, QColor& color)
    {
        if ( stops.empty() )
        {
            index = 0;
            pos = 0;
            color = Qt::black;
            return;
        }
        if ( stops.size() == 1 )
        {
            color = stops[0].second;
            if ( stops[0].first == 1 )
            {
                index = 0;
                pos = 0.5;
            }
            else
            {
                index = 1;
                pos = (stops[0].first + 1) / 2;
            }
            return;
        }

        int i_before = selected;
        if ( i_before == -1 )
            i_before = stops.size() - 1;

        if ( i_before == stops.size() - 1 )
        {
            if ( stops[i_before].first < 1 )
            {
                color = stops[i_before].second;
                pos = (stops[i_before].first + 1) / 2;
                index = stops.size();
                return;
            }
            i_before--;
        }

        index = i_before + 1;
        pos = (stops[i_before].first + stops[i_before+1].first) / 2;
        color = blendColors(stops[i_before].second, stops[i_before+1].second, 0.5);
    }

    void add_color_mouse(QMouseEvent* ev, GradientEditor* parent)
    {
        qreal pos = move_pos(ev->pos(), parent);
        auto info = gradientBlendedColorInsert(stops, pos);
        stops.insert(info.first, info.second);
        selected = highlighted = info.first;
        refresh_gradient();
    }

    void show_dialog_highlighted()
    {
        if ( highlighted == -1 )
            return;

        dialog_selected = highlighted;
        color_dialog.setColor(stops[highlighted].second);
        color_dialog.show();
    }
};

GradientEditor::GradientEditor(QWidget *parent) :
    GradientEditor(Qt::Horizontal, parent)
{}

GradientEditor::GradientEditor(Qt::Orientation orientation, QWidget *parent) :
    QWidget(parent), p(new Private)
{
    p->orientation = orientation;
    setMouseTracking(true);
    resize(sizeHint());
    setAcceptDrops(true);

    p->color_dialog.setParent(this);
    p->color_dialog.setWindowFlags(Qt::Dialog);
    p->color_dialog.setWindowModality(Qt::WindowModal);

    connect(&p->color_dialog, &ColorDialog::colorSelected, this, &GradientEditor::dialogUpdate);
}

GradientEditor::~GradientEditor()
{
    p->color_dialog.setParent(nullptr);
    delete p;
}

void GradientEditor::dialogUpdate(const QColor& c)
{
    if ( p->dialog_selected != -1 )
    {
        p->stops[p->dialog_selected].second = c;
        p->dialog_selected = -1;
        p->refresh_gradient();
        Q_EMIT stopsChanged(p->stops);
        update();
    }
}

void GradientEditor::mouseDoubleClickEvent(QMouseEvent *ev)
{
    if ( ev->button() == Qt::LeftButton )
    {
        ev->accept();
        if ( p->highlighted != -1 )
        {
            qreal highlighted_pos = p->paint_pos(p->stops[p->highlighted], this);
            qreal mouse_pos = orientation() == Qt::Vertical ? ev->pos().y() : ev->pos().x();
            qreal tolerance = 4;
            if ( qAbs(mouse_pos - highlighted_pos) <= tolerance )
            {
                p->show_dialog_highlighted();
                return;
            }
        }

        p->add_color_mouse(ev, this);
        Q_EMIT selectedStopChanged(p->selected);
        update();
    }
    else
    {
        QWidget::mousePressEvent(ev);
    }
}

void GradientEditor::mousePressEvent(QMouseEvent *ev)
{
    if ( ev->button() == Qt::LeftButton )
    {
        ev->accept();
        p->selected = p->highlighted = p->closest(ev->pos(), this);
        emit selectedStopChanged(p->selected);
        update();
    }
    else
    {
        QWidget::mousePressEvent(ev);
    }
}

void GradientEditor::mouseMoveEvent(QMouseEvent *ev)
{
    if ( ev->buttons() & Qt::LeftButton && p->selected != -1 )
    {
        ev->accept();
        qreal pos = p->move_pos(ev->pos(), this);
        if ( p->selected > 0 && pos < p->stops[p->selected-1].first )
        {
            std::swap(p->stops[p->selected], p->stops[p->selected-1]);
            p->selected--;
            Q_EMIT selectedStopChanged(p->selected);
        }
        else if ( p->selected < p->stops.size()-1 && pos > p->stops[p->selected+1].first )
        {
            std::swap(p->stops[p->selected], p->stops[p->selected+1]);
            p->selected++;
            Q_EMIT selectedStopChanged(p->selected);
        }
        p->highlighted = p->selected;
        p->stops[p->selected].first = pos;
        p->refresh_gradient();
        update();
    }
    else
    {
        p->highlighted = p->closest(ev->pos(), this);
        update();
    }
}

void GradientEditor::mouseReleaseEvent(QMouseEvent *ev)
{
    if ( ev->button() == Qt::LeftButton && p->selected != -1 )
    {
        ev->accept();
        QRect bound_rect = rect();
        QPoint localpt = ev->localPos().toPoint();
        const int w_margin = 24;
        const int h_margin = 8;
        bool x_out = localpt.x() < -w_margin || localpt.x() > bound_rect.width() + w_margin;
        bool y_out = localpt.y() < -h_margin || localpt.y() > bound_rect.height() + h_margin;

        if ( p->stops.size() > 1 && (
            (orientation() == Qt::Horizontal && !x_out && y_out) ||
            (orientation() == Qt::Vertical && x_out && !y_out)
        ))
        {
            p->stops.remove(p->selected);
            p->highlighted = p->selected = p->dialog_selected = -1;
            p->refresh_gradient();
            Q_EMIT selectedStopChanged(p->selected);
        }

        Q_EMIT stopsChanged(p->stops);
        update();
    }
    else if ( ev->button() == Qt::RightButton )
    {
        QMenu menu(this);
        menu.addAction(QIcon::fromTheme("list-add"), tr("Add Color"), this, [this, ev]{
            p->add_color_mouse(ev, this);
            Q_EMIT selectedStopChanged(p->selected);
            Q_EMIT stopsChanged(p->stops);
            update();
        });
        if ( p->highlighted != -1 )
        {
            int h = p->highlighted; // leaveEvent resets it when showing the menu
            menu.addAction(QIcon::fromTheme("list-remove"), tr("Remove Color"), this, [this, h]{
                p->stops.remove(h);
                p->highlighted = -1;
                p->refresh_gradient();
                Q_EMIT selectedStopChanged(p->selected);
                Q_EMIT stopsChanged(p->stops);
                update();
            });
            menu.addAction(QIcon::fromTheme("document-edit"), tr("Edit Color..."), this, [this, h]{
                p->highlighted = h;
                p->show_dialog_highlighted();
            });
        }

        menu.exec(ev->globalPos());
    }
    else
    {
        QWidget::mousePressEvent(ev);
    }
}

void GradientEditor::leaveEvent(QEvent*)
{
    p->highlighted = -1;
    update();
}


QBrush GradientEditor::background() const
{
    return p->back;
}

void GradientEditor::setBackground(const QBrush &bg)
{
    p->back = bg;
    update();
    Q_EMIT backgroundChanged(bg);
}

QGradientStops GradientEditor::stops() const
{
    return p->stops;
}

void GradientEditor::setStops(const QGradientStops &colors)
{
    p->selected = p->highlighted = p->dialog_selected = -1;
    p->stops = colors;
    p->refresh_gradient();
    emit selectedStopChanged(p->selected);
    emit stopsChanged(p->stops);
    update();
}

QLinearGradient GradientEditor::gradient() const
{
    return p->gradient;
}

void GradientEditor::setGradient(const QLinearGradient &gradient)
{
    setStops(gradient.stops());
}

Qt::Orientation GradientEditor::orientation() const
{
    return p->orientation;
}

void GradientEditor::setOrientation(Qt::Orientation orientation)
{
    p->orientation = orientation;
    update();
}


void GradientEditor::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    QStyleOptionFrame panel;
    panel.initFrom(this);
    panel.lineWidth = 1;
    panel.midLineWidth = 0;
    panel.state |= QStyle::State_Sunken;
    style()->drawPrimitive(QStyle::PE_Frame, &panel, &painter, this);
    QRect r = style()->subElementRect(QStyle::SE_FrameContents, &panel, this);
    painter.setClipRect(r);


    if(orientation() == Qt::Horizontal)
        p->gradient.setFinalStop(1, 0);
    else
        p->gradient.setFinalStop(0, -1);

    painter.setPen(Qt::NoPen);
    painter.setBrush(p->back);
    painter.drawRect(1,1,geometry().width()-2,geometry().height()-2);
    painter.setBrush(p->gradient);
    painter.drawRect(1,1,geometry().width()-2,geometry().height()-2);

    /// \todo Take orientation into account
    int i = 0;
    for ( const QGradientStop& stop : p->stops )
    {
        QColor color = stop.second;
        Qt::GlobalColor border_color = Qt::black;
        Qt::GlobalColor core_color = Qt::white;

        if ( color.valueF() <= 0.5 && color.alphaF() >= 0.5 )
            std::swap(core_color, border_color);

        QPointF p1 = QPointF(p->paint_pos(stop, this), 2.5);
        QPointF p2 = p1 + QPointF(0, geometry().height() - 5);
        if ( i == p->selected )
        {
            painter.setPen(QPen(border_color, 5));
            painter.drawLine(p1, p2);
            painter.setPen(QPen(core_color, 3));
            painter.drawLine(p1, p2);
        }
        else if ( i == p->highlighted )
        {
            painter.setPen(QPen(border_color, 3));
            painter.drawLine(p1, p2);
            painter.setPen(QPen(core_color, 1));
            painter.drawLine(p1, p2);
        }
        else
        {
            painter.setPen(QPen(border_color, 3));
            painter.drawLine(p1, p2);
        }

        i++;
    }

    if ( p->drop_index != -1 && p->drop_color.isValid() )
    {
        qreal pos = p->drop_pos * (geometry().width() - 5);
        painter.setPen(QPen(p->drop_color, 3));
        QPointF p1 = QPointF(2.5, 2.5) + QPointF(pos, 0);
        QPointF p2 = p1 + QPointF(0, geometry().height() - 5);
        painter.drawLine(p1, p2);
    }

}

QSize GradientEditor::sizeHint() const
{
    QStyleOptionSlider opt;
    opt.orientation = p->orientation;

    int w = style()->pixelMetric(QStyle::PM_SliderThickness, &opt, this);
    int h = std::max(84, style()->pixelMetric(QStyle::PM_SliderLength, &opt, this));
    if ( p->orientation == Qt::Horizontal )
    {
        std::swap(w, h);
    }
    QSlider s;
    return style()->sizeFromContents(QStyle::CT_Slider, &opt, QSize(w, h), &s)
        .expandedTo(QApplication::globalStrut());
}

int GradientEditor::selectedStop() const
{
    return p->selected;
}

void GradientEditor::setSelectedStop(int stop)
{
    if ( stop >= -1 && stop < p->stops.size() )
    {
        p->selected = stop;
        emit selectedStopChanged(p->selected);
    }
}

QColor GradientEditor::selectedColor() const
{
    if ( p->selected != -1 )
        return p->stops[p->selected].second;
    return {};
}

void GradientEditor::setSelectedColor(const QColor& color)
{
    if ( p->selected != -1 )
    {
        p->stops[p->selected].second = color;
        p->refresh_gradient();
        update();
    }
}


void GradientEditor::dragEnterEvent(QDragEnterEvent *event)
{
    p->drop_event(event, this);

    if ( p->drop_color.isValid() && p->drop_index != -1 )
    {
        event->setDropAction(Qt::CopyAction);
        event->accept();
    }
}

void GradientEditor::dragMoveEvent(QDragMoveEvent* event)
{
    p->drop_event(event, this);
}

void GradientEditor::dragLeaveEvent(QDragLeaveEvent *)
{
    p->clear_drop(this);
}

void GradientEditor::dropEvent(QDropEvent *event)
{
    p->drop_event(event, this);

    if ( !p->drop_color.isValid() || p->drop_index == -1 )
        return;

    p->stops.insert(p->drop_index, {p->drop_pos, p->drop_color});
    p->refresh_gradient();
    p->selected = p->drop_index;
    event->accept();
    p->clear_drop(this);
    emit selectedStopChanged(p->selected);
}

void GradientEditor::addStop()
{
    int index = -1;
    qreal pos = 0;
    QColor color;
    p->add_stop_data(index, pos, color);
    p->stops.insert(index, {pos, color});
    p->selected = p->highlighted = index;
    p->refresh_gradient();
    update();
    emit selectedStopChanged(p->selected);
}

void GradientEditor::removeStop()
{
    if ( p->stops.size() < 2 )
        return;

    int selected = p->selected;
    if ( selected == -1 )
        selected = p->stops.size() - 1;
    p->stops.remove(selected);
    p->refresh_gradient();

    if ( p->selected != -1 )
    {
        p->selected = -1;
        emit selectedStopChanged(p->selected);
    }

    p->dialog_selected = -1;

    update();

}

ColorDialog * GradientEditor::dialog() const
{
    return &p->color_dialog;
}


} // namespace color_widgets
