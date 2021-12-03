/**
 * \file
 *
 * \author Mattia Basaglia
 *
 * \copyright Copyright (C) 2013-2020 Mattia Basaglia
 * \copyright Copyright (C) 2014 Calle Laakkonen
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
#include "hue_slider_plugin.hpp"
#include "QtColorWidgets/hue_slider.hpp"
#include <QtPlugin>

HueSlider_Plugin::HueSlider_Plugin(QObject *parent)
    : QObject(parent), initialized(false)
{
}


void HueSlider_Plugin::initialize(QDesignerFormEditorInterface *)
{
    if (initialized)
        return;

    initialized = true;
}

bool HueSlider_Plugin::isInitialized() const
{
    return initialized;
}

QWidget *HueSlider_Plugin::createWidget(QWidget *parent)
{
    return new color_widgets::HueSlider(parent);
}

QString HueSlider_Plugin::name() const
{
    return "color_widgets::HueSlider";
}

QString HueSlider_Plugin::group() const
{
    return "Color Widgets";
}

QIcon HueSlider_Plugin::icon() const
{
    color_widgets::HueSlider w;
    w.resize(64,16);
    QPixmap pix(64,64);
    pix.fill(Qt::transparent);
    w.render(&pix,QPoint(0,16));
    return QIcon(pix);
}

QString HueSlider_Plugin::toolTip() const
{
    return "Slider over a hue gradient";
}

QString HueSlider_Plugin::whatsThis() const
{
    return toolTip();
}

bool HueSlider_Plugin::isContainer() const
{
    return false;
}

QString HueSlider_Plugin::domXml() const
{

    return "<ui language=\"c++\">\n"
           " <widget class=\"color_widgets::HueSlider\" name=\"HueSlider\">\n"
           " </widget>\n"
            "</ui>\n";
}

QString HueSlider_Plugin::includeFile() const
{
    return "QtColorWidgets/hue_slider.hpp";
}
