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
#include "color_widget_plugin_collection.hpp"
#include "color_preview_plugin.hpp"
#include "color_wheel_plugin.hpp"
#include "gradient_slider_plugin.hpp"
#include "hue_slider_plugin.hpp"
#include "color_selector_plugin.hpp"
#include "color_list_plugin.hpp"
#include "swatch_plugin.hpp"
#include "color_palette_widget_plugin.hpp"
#include "color_2d_slider_plugin.hpp"
#include "color_line_edit_plugin.hpp"
#include "gradient_editor_plugin.hpp"
// add new plugin headers above this line

ColorWidgets_PluginCollection::ColorWidgets_PluginCollection(QObject *parent) :
    QObject(parent)
{
    widgets.push_back(new ColorPreview_Plugin(this));
    widgets.push_back(new ColorWheel_Plugin(this));
    widgets.push_back(new GradientSlider_Plugin(this));
    widgets.push_back(new HueSlider_Plugin(this));
    widgets.push_back(new ColorSelector_Plugin(this));
    widgets.push_back(new ColorListWidget_Plugin(this));
    widgets.push_back(new Swatch_Plugin(this));
    widgets.push_back(new ColorPaletteWidget_Plugin(this));
    widgets.push_back(new Color2DSlider_Plugin(this));
    widgets.push_back(new ColorLineEdit_Plugin(this));
    widgets.push_back(new GradientEditor_Plugin(this));
    // add new plugins above this line
}

QList<QDesignerCustomWidgetInterface *> ColorWidgets_PluginCollection::customWidgets() const
{
    return widgets;
}
