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
#include "color_list_plugin.hpp"
#include "QtColorWidgets/color_list_widget.hpp"

ColorListWidget_Plugin::ColorListWidget_Plugin(QObject *parent) :
    QObject(parent)
{
}


void ColorListWidget_Plugin::initialize(QDesignerFormEditorInterface *)
{
    initialized = true;
}

bool ColorListWidget_Plugin::isInitialized() const
{
    return initialized;
}

QWidget *ColorListWidget_Plugin::createWidget(QWidget *parent)
{
    return new color_widgets::ColorListWidget(parent);
}

QString ColorListWidget_Plugin::name() const
{
    return "color_widgets::ColorListWidget";
}

QString ColorListWidget_Plugin::group() const
{
    return "Color Widgets";
}

QIcon ColorListWidget_Plugin::icon() const
{
    return QIcon::fromTheme("format-stroke-color");
}

QString ColorListWidget_Plugin::toolTip() const
{
    return "An editable list of colors";
}

QString ColorListWidget_Plugin::whatsThis() const
{
    return toolTip();
}

bool ColorListWidget_Plugin::isContainer() const
{
    return false;
}

QString ColorListWidget_Plugin::domXml() const
{

    return "<ui language=\"c++\">\n"
           " <widget class=\"color_widgets::ColorListWidget\" name=\"ColorListWidget\">\n"
           " </widget>\n"
            "</ui>\n";
}

QString ColorListWidget_Plugin::includeFile() const
{
    return "QtColorWidgets/color_list_widget.hpp";
}
