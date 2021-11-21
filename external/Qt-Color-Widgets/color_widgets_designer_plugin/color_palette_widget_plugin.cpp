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
#include "color_palette_widget_plugin.hpp"
#include "QtColorWidgets/color_palette_widget.hpp"

ColorPaletteWidget_Plugin::ColorPaletteWidget_Plugin(QObject *parent) :
    QObject(parent), initialized(false)
{
}

void ColorPaletteWidget_Plugin::initialize(QDesignerFormEditorInterface *)
{
    initialized = true;
}

bool ColorPaletteWidget_Plugin::isInitialized() const
{
    return initialized;
}

QWidget* ColorPaletteWidget_Plugin::createWidget(QWidget *parent)
{
    color_widgets::ColorPaletteWidget *wid = new color_widgets::ColorPaletteWidget(parent);

    color_widgets::ColorPalette palette1;
    color_widgets::ColorPalette palette2;
    int columns = 12;
    palette1.setName("Palette 1");
    palette2.setName("Palette 2");
    palette1.setColumns(columns);
    palette2.setColumns(columns);
    for ( int i = 0; i < 6; i++ )
    {
        for ( int j = 0; j < columns; j++ )
        {
            float f = float(j)/columns;
            palette1.appendColor(QColor::fromHsvF(i/8.0,1-f,0.5+f/2));
            palette2.appendColor(QColor::fromHsvF(i/8.0,1-f,1-f));
        }
    }
    color_widgets::ColorPaletteModel *model = new color_widgets::ColorPaletteModel;
    model->setParent(wid);
    model->addPalette(palette1, false);
    model->addPalette(palette2, false);
    wid->setModel(model);

    return wid;
}

QString ColorPaletteWidget_Plugin::name() const
{
    return "color_widgets::ColorPaletteWidget";
}

QString ColorPaletteWidget_Plugin::group() const
{
    return "Color Widgets";
}

QIcon ColorPaletteWidget_Plugin::icon() const
{
    color_widgets::ColorPalette w;
    w.setColumns(6);
    for ( int i = 0; i < 4; i++ )
    {
        for ( int j = 0; j < w.columns(); j++ )
        {
            float f = float(j)/w.columns();
            w.appendColor(QColor::fromHsvF(i/5.0,1-f,0.5+f/2));
        }
    }
    return QIcon(w.preview(QSize(64,64)));
}

QString ColorPaletteWidget_Plugin::toolTip() const
{
    return "A widget that displays a color palette";
}

QString ColorPaletteWidget_Plugin::whatsThis() const
{
    return toolTip();
}

bool ColorPaletteWidget_Plugin::isContainer() const
{
    return false;
}

QString ColorPaletteWidget_Plugin::domXml() const
{
    return "<ui language=\"c++\">\n"
           " <widget class=\"color_widgets::ColorPaletteWidget\" name=\"palette_widget\">\n"
           " </widget>\n"
           "</ui>\n";
}

QString ColorPaletteWidget_Plugin::includeFile() const
{
    return "QtColorWidgets/color_palette_widget.hpp";
}
