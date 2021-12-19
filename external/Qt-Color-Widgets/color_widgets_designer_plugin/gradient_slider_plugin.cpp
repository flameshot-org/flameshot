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
#include "gradient_slider_plugin.hpp"
#include "QtColorWidgets/gradient_slider.hpp"
#include <QtPlugin>

GradientSlider_Plugin::GradientSlider_Plugin(QObject *parent)
    : QObject(parent), initialized(false)
{
}


void GradientSlider_Plugin::initialize(QDesignerFormEditorInterface *)
{
    if (initialized)
        return;

    initialized = true;
}

bool GradientSlider_Plugin::isInitialized() const
{
    return initialized;
}

QWidget *GradientSlider_Plugin::createWidget(QWidget *parent)
{
    return new color_widgets::GradientSlider(parent);
}

QString GradientSlider_Plugin::name() const
{
    return "color_widgets::GradientSlider";
}

QString GradientSlider_Plugin::group() const
{
    return "Color Widgets";
}

QIcon GradientSlider_Plugin::icon() const
{
    color_widgets::GradientSlider w;
    w.resize(64,16);
    QVector<QColor> cols;
    cols.push_back(Qt::green);
    cols.push_back(Qt::yellow);
    cols.push_back(Qt::red);
    w.setColors(cols);
    QPixmap pix(64,64);
    pix.fill(Qt::transparent);
    w.render(&pix,QPoint(0,16));
    return QIcon(pix);
}

QString GradientSlider_Plugin::toolTip() const
{
    return "Slider over a gradient";
}

QString GradientSlider_Plugin::whatsThis() const
{
    return toolTip();
}

bool GradientSlider_Plugin::isContainer() const
{
    return false;
}

QString GradientSlider_Plugin::domXml() const
{

    return "<ui language=\"c++\">\n"
           " <widget class=\"color_widgets::GradientSlider\" name=\"GradientSlider\">\n"
           " </widget>\n"
            "</ui>\n";
}

QString GradientSlider_Plugin::includeFile() const
{
    return "QtColorWidgets/gradient_slider.hpp";
}
