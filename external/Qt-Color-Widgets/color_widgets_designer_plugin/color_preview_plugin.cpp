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
#include "color_preview_plugin.hpp"
#include "QtColorWidgets/color_preview.hpp"
#include <QtPlugin>


ColorPreview_Plugin::ColorPreview_Plugin(QObject *parent)
    : QObject(parent), initialized(false)
{
}


void ColorPreview_Plugin::initialize(QDesignerFormEditorInterface *)
{
    if (initialized)
        return;

    initialized = true;
}

bool ColorPreview_Plugin::isInitialized() const
{
    return initialized;
}

QWidget *ColorPreview_Plugin::createWidget(QWidget *parent)
{
    return new color_widgets::ColorPreview(parent);
}

QString ColorPreview_Plugin::name() const
{
    return "color_widgets::ColorPreview";
}

QString ColorPreview_Plugin::group() const
{
    return "Color Widgets";
}

QIcon ColorPreview_Plugin::icon() const
{
    return QIcon();
}

QString ColorPreview_Plugin::toolTip() const
{
    return "Display a color";
}

QString ColorPreview_Plugin::whatsThis() const
{
    return toolTip();
}

bool ColorPreview_Plugin::isContainer() const
{
    return false;
}

QString ColorPreview_Plugin::domXml() const
{

    return "<ui language=\"c++\">\n"
           " <widget class=\"color_widgets::ColorPreview\" name=\"colorPreview\">\n"
           " </widget>\n"
            "</ui>\n";
}

QString ColorPreview_Plugin::includeFile() const
{
    return "QtColorWidgets/color_preview.hpp";
}

//Q_EXPORT_PLUGIN2(color_widgets, ColorPreview_Plugin);
