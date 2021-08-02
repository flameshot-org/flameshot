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
#include "gradient_editor_plugin.hpp"
#include "QtColorWidgets/gradient_editor.hpp"

QWidget* GradientEditor_Plugin::createWidget(QWidget *parent)
{
    color_widgets::GradientEditor *widget = new color_widgets::GradientEditor(parent);
    return widget;
}

QIcon GradientEditor_Plugin::icon() const
{
    color_widgets::GradientEditor w;
    w.resize(64,16);
    QGradientStops cols;
    cols.push_back({0.2, Qt::green});
    cols.push_back({0.5, Qt::yellow});
    cols.push_back({0.8, Qt::red});
    w.setStops(cols);
    QPixmap pix(64,64);
    pix.fill(Qt::transparent);
    w.render(&pix, QPoint(0,16));
    return QIcon(pix);
}

QString GradientEditor_Plugin::domXml() const
{
    return "<ui language=\"c++\">\n"
           " <widget class=\"color_widgets::GradientEditor\" name=\"gradient_editor\">\n"
           " </widget>\n"
           "</ui>\n";
}

bool GradientEditor_Plugin::isContainer() const
{
    return false;
}

GradientEditor_Plugin::GradientEditor_Plugin(QObject *parent) :
    QObject(parent), initialized(false)
{
}

void GradientEditor_Plugin::initialize(QDesignerFormEditorInterface *)
{
    initialized = true;
}

bool GradientEditor_Plugin::isInitialized() const
{
    return initialized;
}

QString GradientEditor_Plugin::name() const
{
    return "color_widgets::GradientEditor";
}

QString GradientEditor_Plugin::group() const
{
    return "Color Widgets";
}

QString GradientEditor_Plugin::toolTip() const
{
    return "Widget to edit gradient stops";
}

QString GradientEditor_Plugin::whatsThis() const
{
    return toolTip();
}

QString GradientEditor_Plugin::includeFile() const
{
    return "QtColorWidgets/gradient_editor.hpp";
}

