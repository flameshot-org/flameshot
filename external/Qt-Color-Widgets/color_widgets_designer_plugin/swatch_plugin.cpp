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
#include "swatch_plugin.hpp"
#include "QtColorWidgets/swatch.hpp"

Swatch_Plugin::Swatch_Plugin(QObject *parent) :
    QObject(parent), initialized(false)
{
}

void Swatch_Plugin::initialize(QDesignerFormEditorInterface *)
{
    initialized = true;
}

bool Swatch_Plugin::isInitialized() const
{
    return initialized;
}

QWidget* Swatch_Plugin::createWidget(QWidget *parent)
{
    color_widgets::Swatch *wid = new color_widgets::Swatch(parent);
    wid->palette().setColumns(12);
    for ( int i = 0; i < 6; i++ )
    {
        for ( int j = 0; j < wid->palette().columns(); j++ )
        {
            float f = float(j)/wid->palette().columns();
            wid->palette().appendColor(QColor::fromHsvF(i/8.0,1-f,0.5+f/2));
        }
    }
    return wid;
}

QString Swatch_Plugin::name() const
{
    return "color_widgets::Swatch";
}

QString Swatch_Plugin::group() const
{
    return "Color Widgets";
}

QIcon Swatch_Plugin::icon() const
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

QString Swatch_Plugin::toolTip() const
{
    return "A widget that displays a color palette";
}

QString Swatch_Plugin::whatsThis() const
{
    return toolTip();
}

bool Swatch_Plugin::isContainer() const
{
    return false;
}

QString Swatch_Plugin::domXml() const
{
    return "<ui language=\"c++\">\n"
           " <widget class=\"color_widgets::Swatch\" name=\"swatch\">\n"
           " </widget>\n"
           "</ui>\n";
}

QString Swatch_Plugin::includeFile() const
{
    return "QtColorWidgets/swatch.hpp";
}
