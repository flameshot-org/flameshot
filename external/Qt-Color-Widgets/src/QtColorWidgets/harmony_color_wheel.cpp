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
#include "QtColorWidgets/color_wheel_private.hpp"
#include "QtColorWidgets/harmony_color_wheel.hpp"

namespace color_widgets {

struct RingEditor
{
    double hue_diff;
    bool editable;
    int symmetric_to;
    int opposite_to;
    RingEditor(double hue_diff, bool editable, int symmetric_to=-1, int opposite_to=-1) :
        hue_diff(hue_diff),
        editable(editable),
        symmetric_to(symmetric_to),
        opposite_to(opposite_to)
    {
    }
};

class HarmonyColorWheel::Private : public ColorWheel::Private
{
public:
    using ColorWheel::Private::Private;

    std::vector<RingEditor> ring_editors;
    int current_ring_editor = -1;

    /**
     * Puts a double into [0; 1) range
     */
    static inline double normalize(double angle)
    {
        return angle - std::floor(angle);
    }
};


HarmonyColorWheel::HarmonyColorWheel(QWidget *parent) :
    ColorWheel(parent, new Private(this))
{
    connect(this, SIGNAL(colorChanged(QColor)), this, SIGNAL(harmonyChanged()));
    p = static_cast<HarmonyColorWheel::Private*>(data());
}

HarmonyColorWheel::~HarmonyColorWheel() = default;

QList<QColor> HarmonyColorWheel::harmonyColors() const
{
    QList<QColor> result;
    result.push_back(color());
    for (auto const& harmony : p->ring_editors)
    {
        auto hue = Private::normalize(p->hue+harmony.hue_diff);
        result.push_back(p->color_from(hue, p->sat, p->val, 1));
    }
    return result;
}

unsigned int HarmonyColorWheel::harmonyCount() const
{
    return 1 + p->ring_editors.size();
}

void HarmonyColorWheel::clearHarmonies()
{
    p->ring_editors.clear();
    p->current_ring_editor = -1;
    Q_EMIT harmonyChanged();
    update();
}

unsigned HarmonyColorWheel::addHarmony(double hue_diff, bool editable)
{
    auto count = p->ring_editors.size();
    p->ring_editors.emplace_back(Private::normalize(hue_diff), editable, -1, -1);
    Q_EMIT harmonyChanged();
    update();
    return count;
}

unsigned HarmonyColorWheel::addSymmetricHarmony(unsigned relative_to)
{
    auto count = p->ring_editors.size();
    if (relative_to >= count)
        throw std::out_of_range("incorrect call to addSymmetricHarmony: harmony number out of range");
    auto& relative = p->ring_editors[relative_to];
    relative.symmetric_to = count;
    p->ring_editors.emplace_back(Private::normalize(-relative.hue_diff), relative.editable, relative_to, -1);
    Q_EMIT harmonyChanged();
    update();
    return count;
}

unsigned HarmonyColorWheel::addOppositeHarmony(unsigned relative_to)
{
    auto count = p->ring_editors.size();
    if (relative_to >= count)
        throw std::out_of_range("incorrect call to addOppositeHarmony: harmony number out of range");
    auto& relative = p->ring_editors[relative_to];
    relative.opposite_to = count;
    p->ring_editors.emplace_back(Private::normalize(0.5+relative.hue_diff), relative.editable, -1, relative_to);
    Q_EMIT harmonyChanged();
    update();
    return count;
}

void HarmonyColorWheel::paintEvent(QPaintEvent * ev)
{
    ColorWheel::paintEvent(ev);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.translate(geometry().width()/2,geometry().height()/2);

    for (auto const& editor : p->ring_editors)
    {
        auto hue = p->hue+editor.hue_diff;
        // TODO: better color for uneditable indicator
        auto color = editor.editable ? Qt::white : Qt::gray;
        p->draw_ring_editor(hue, painter, color);
    }
}


void HarmonyColorWheel::mousePressEvent(QMouseEvent *ev)
{
    if ( ev->buttons() & Qt::LeftButton )
    {
        QLineF ray = p->line_to_point(ev->pos());
        if ( ray.length() <= p->outer_radius() &&  ray.length() > p->inner_radius() )
        {
            p->mouse_status = DragCircle;
            auto hue_diff = Private::normalize(ray.angle()/360 - p->hue);
            auto i = 0;
            for (auto const& editor : p->ring_editors)
            {
                const double eps = 1.0/64;
                if (editor.editable &&
                    editor.hue_diff <= hue_diff + eps &&
                    editor.hue_diff >= hue_diff - eps)
                {
                    p->current_ring_editor = i;
                    // no need to update color..
                    return;
                }
                ++i;
            }
        }
    }
    ColorWheel::mousePressEvent(ev);
}


void HarmonyColorWheel::mouseMoveEvent(QMouseEvent *ev)
{
    if ( p->mouse_status == DragCircle && p->current_ring_editor != -1 )
    {
        auto hue = p->line_to_point(ev->pos()).angle()/360.0;
        auto& editor = p->ring_editors[p->current_ring_editor];
        editor.hue_diff = Private::normalize(hue - p->hue);
        if (editor.symmetric_to != -1)
        {
            auto& symmetric = p->ring_editors[editor.symmetric_to];
            symmetric.hue_diff = Private::normalize(p->hue - hue);
        }
        else if (editor.opposite_to != -1)
        {
            auto& opposite = p->ring_editors[editor.opposite_to];
            opposite.hue_diff = Private::normalize(editor.hue_diff-0.5);
        }
        Q_EMIT harmonyChanged();
        update();
        return;
    }
    ColorWheel::mouseMoveEvent(ev);
}

void HarmonyColorWheel::mouseReleaseEvent(QMouseEvent *ev)
{
    ColorWheel::mouseReleaseEvent(ev);
    p->current_ring_editor = -1;
}

} // namespace color_widgets
