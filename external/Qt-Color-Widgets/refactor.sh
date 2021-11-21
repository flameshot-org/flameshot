#!/bin/bash
#
# Copyright (C) 2013-2020 Mattia Basaglia
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

################################################################################
# This script is to refactor the old class names to the new ones               #
# eg: occurrences of Color_Dialog become color_widgets::ColorDialog            #
# This script does very simple text replacements and overwrites existing files #
# Use with care                                                                #
# Usage:                                                                       #
#   ./refactor.sh /path/to/sources                                             #
#                                                                              #
################################################################################

old_classes=(
    Color_Delegate
    Color_Dialog
    Color_List_Widget
    Color_Preview
    Color_Selector
    Color_Wheel
    Gradient_Slider
    Hue_Slider
)
old_enums=(
    Button_Mode
    Display_Mode
    Update_Mode
    Display_Enum
    Display_Flags
)
file_extensions=(
    ui
    cpp
    hpp
    C
    H
    h
    cxx
    hxx
)


function new_class_name()
{
    echo "$1" | sed -e 's/_//g' -r -e 's/^/color_widgets::/'
}


function new_enum_name()
{
    echo "$1" | sed -e 's/_//g'
}

directory="$1"

if [ -z "$directory" ]
then
    echo "Usage: $0 (directory)"
    exit 1
fi

find_extensions=""
for ext in ${file_extensions[@]}
do
    find_extensions="$find_extensions -o -name '*.$ext'"
done
find_extensions="$(echo "$find_extensions" | sed -r 's/ -o //')"
find_command="find \""$directory"\" -type f -a \( $find_extensions \) -print"

files="$(bash -c "$find_command")"

replacements=""
for class in ${old_classes[@]}
do
    replacements="$replacements $class $(new_class_name $class)"
done
for enum in ${old_enums[@]}
do
    replacements="$replacements $enum $(new_enum_name $enum)"
done

for file in $files
do
   replace $replacements -- "$file"
done
