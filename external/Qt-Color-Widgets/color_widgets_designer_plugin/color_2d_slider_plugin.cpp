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
#include "color_2d_slider_plugin.hpp"
#include "QtColorWidgets/color_2d_slider.hpp"

Color2DSlider_Plugin::Color2DSlider_Plugin(QObject *parent) :
    QObject(parent), initialized(false)
{
}

void Color2DSlider_Plugin::initialize(QDesignerFormEditorInterface *)
{
    initialized = true;
}

bool Color2DSlider_Plugin::isInitialized() const
{
    return initialized;
}

QWidget *Color2DSlider_Plugin::createWidget(QWidget *parent)
{
    return new color_widgets::Color2DSlider(parent);
}

QString Color2DSlider_Plugin::name() const
{
    return "color_widgets::Color2DSlider";
}

QString Color2DSlider_Plugin::group() const
{
    return "Color Widgets";
}

QIcon Color2DSlider_Plugin::icon() const
{
    color_widgets::Color2DSlider w;
    w.resize(64,64);
    QPixmap pix(64,64);
    w.render(&pix);
    return QIcon(pix);
}

QString Color2DSlider_Plugin::toolTip() const
{
    return "An analog widget to select 2 color components at the same time";
}

QString Color2DSlider_Plugin::whatsThis() const
{
    return toolTip();
}

bool Color2DSlider_Plugin::isContainer() const
{
    return false;
}

QString Color2DSlider_Plugin::domXml() const
{
    return "<ui language=\"c++\">\n"
           " <widget class=\"color_widgets::Color2DSlider\" name=\"color2DSlider\">\n"
           " </widget>\n"
           "</ui>\n";
}

QString Color2DSlider_Plugin::includeFile() const
{
    return "QtColorWidgets/color_2d_slider.hpp";
}
