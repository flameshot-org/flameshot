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
#include "color_line_edit_plugin.hpp"
#include "QtColorWidgets/color_line_edit.hpp"

QWidget* ColorLineEdit_Plugin::createWidget(QWidget *parent)
{
    color_widgets::ColorLineEdit *widget = new color_widgets::ColorLineEdit(parent);
    return widget;
}

QIcon ColorLineEdit_Plugin::icon() const
{
    return QIcon::fromTheme("edit-rename");
}

QString ColorLineEdit_Plugin::domXml() const
{
    return "<ui language=\"c++\">\n"
           " <widget class=\"color_widgets::ColorLineEdit\" name=\"color_line_edit\">\n"
           " </widget>\n"
           "</ui>\n";
}

bool ColorLineEdit_Plugin::isContainer() const
{
    return false;
}

ColorLineEdit_Plugin::ColorLineEdit_Plugin(QObject *parent) :
    QObject(parent), initialized(false)
{
}

void ColorLineEdit_Plugin::initialize(QDesignerFormEditorInterface *)
{
    initialized = true;
}

bool ColorLineEdit_Plugin::isInitialized() const
{
    return initialized;
}

QString ColorLineEdit_Plugin::name() const
{
    return "color_widgets::ColorLineEdit";
}

QString ColorLineEdit_Plugin::group() const
{
    return "Color Widgets";
}

QString ColorLineEdit_Plugin::toolTip() const
{
    return "A widget to manipulate a string representing a color";
}

QString ColorLineEdit_Plugin::whatsThis() const
{
    return toolTip();
}

QString ColorLineEdit_Plugin::includeFile() const
{
    return "QtColorWidgets/color_line_edit.hpp";
}
