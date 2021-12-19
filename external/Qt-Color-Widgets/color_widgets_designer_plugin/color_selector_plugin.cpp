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
#include "color_selector_plugin.hpp"
#include "QtColorWidgets/color_selector.hpp"
#include <QtPlugin>

ColorSelector_Plugin::ColorSelector_Plugin(QObject *parent)
    : QObject(parent), initialized(false)
{
}


void ColorSelector_Plugin::initialize(QDesignerFormEditorInterface *)
{
    if (initialized)
        return;

    initialized = true;
}

bool ColorSelector_Plugin::isInitialized() const
{
    return initialized;
}

QWidget *ColorSelector_Plugin::createWidget(QWidget *parent)
{
    return new color_widgets::ColorSelector(parent);
}

QString ColorSelector_Plugin::name() const
{
    return "color_widgets::ColorSelector";
}

QString ColorSelector_Plugin::group() const
{
    return "Color Widgets";
}

QIcon ColorSelector_Plugin::icon() const
{
    return QIcon();
}

QString ColorSelector_Plugin::toolTip() const
{
    return "Display a color and opens a color dialog on click";
}

QString ColorSelector_Plugin::whatsThis() const
{
    return toolTip();
}

bool ColorSelector_Plugin::isContainer() const
{
    return false;
}

QString ColorSelector_Plugin::domXml() const
{

    return "<ui language=\"c++\">\n"
           " <widget class=\"color_widgets::ColorSelector\" name=\"ColorSelector\">\n"
           " </widget>\n"
            "</ui>\n";
}

QString ColorSelector_Plugin::includeFile() const
{
    return "QtColorWidgets/color_selector.hpp";
}
