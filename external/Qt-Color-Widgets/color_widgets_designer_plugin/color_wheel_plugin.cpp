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
#include "color_wheel_plugin.hpp"
#include "QtColorWidgets/color_wheel.hpp"

ColorWheel_Plugin::ColorWheel_Plugin(QObject *parent) :
    QObject(parent), initialized(false)
{
}

void ColorWheel_Plugin::initialize(QDesignerFormEditorInterface *)
{
    initialized = true;
}

bool ColorWheel_Plugin::isInitialized() const
{
    return initialized;
}

QWidget *ColorWheel_Plugin::createWidget(QWidget *parent)
{
    return new color_widgets::ColorWheel(parent);
}

QString ColorWheel_Plugin::name() const
{
    return "color_widgets::ColorWheel";
}

QString ColorWheel_Plugin::group() const
{
    return "Color Widgets";
}

QIcon ColorWheel_Plugin::icon() const
{
    color_widgets::ColorWheel w;
    w.resize(64,64);
    w.setWheelWidth(8);
    QPixmap pix(64,64);
    w.render(&pix);
    return QIcon(pix);
}

QString ColorWheel_Plugin::toolTip() const
{
    return "A widget that allows an intuitive selection of HSL parameters for a QColor";
}

QString ColorWheel_Plugin::whatsThis() const
{
    return toolTip();
}

bool ColorWheel_Plugin::isContainer() const
{
    return false;
}

QString ColorWheel_Plugin::domXml() const
{
    return "<ui language=\"c++\">\n"
           " <widget class=\"color_widgets::ColorWheel\" name=\"colorWheel\">\n"
           "  <property name=\"sizePolicy\">\n"
           "   <sizepolicy hsizetype=\"Minimum\" vsizetype=\"Minimum\">\n"
           "    <horstretch>0</horstretch>\n"
           "    <verstretch>0</verstretch>\n"
           "   </sizepolicy>\n"
           "  </property>\n"
           " </widget>\n"
           "</ui>\n";
}

QString ColorWheel_Plugin::includeFile() const
{
    return "QtColorWidgets/color_wheel.hpp";
}
