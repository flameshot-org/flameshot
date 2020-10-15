Color Widgets
=============

Here is a color dialog that is more user-friendly than the default QColorDialog
and several other color-related widgets

The provided widgets are:

* ColorWheel,         An analog widget used to select a color
* ColorPreview,       A simple widget that displays a color
* GradientSlider,     A slider that has a gradient background
* HueSlider,          A variant of GradientSlider that has a rainbow background
* ColorSelector,      A ColorPreview that shows a ColorDialog when clicked
* ColorDialog,        A dialog that uses the above widgets to provide a better user experience than QColorDialog
* ColorListWidget,    A widget to edit a list of colors
* Swatch,             A widget to display a color palette
* ColorPaletteWidget, A widget to use and manage a list of palettes
* Color2DSlider,      An analog widget used to select 2 color components
* ColorLineEdit,      A widget to manipulate a string representing a color

they are all in the color_widgets namespace.

See [the gallery](gallery/README.md) for more information and screenshots.


Using it in a project
---------------------

For QMake-based projects, include color_widgets.pri in the QMake project file.
For CMake-based projects, add this as subdirectory, it will be compiled as a
library and you can link the required targets to ColorWidgets-qt5.
All the required files are in ./src and ./include.


Installing as a Qt Designer/Creator Plugin
------------------------------------------

The sources for the designer plugin are in ./color_widgets_designer_plugin

Compile the library and install in
(Qt SDK)/Tools/QtCreator/bin/designer/
(Qt SDK)/(Qt Version)/(Toolchain)/plugins/designer

cd build && cmake .. && make ColorWidgetsPlugin && make install


Latest Version
--------------

The latest version of the sources can be found at the following locations:

* https://github.com/mbasaglia/Qt-Color-Widgets
* git://github.com/mbasaglia/Qt-Color-Widgets.git


License
-------

LGPLv3+, See COPYING.
As a special exception, this library can be included in any project under the
terms of any of the GNU liceses, distributing the whole project under a
different GNU license, see LICENSE-EXCEPTION for details.

Copyright (C) 2013-2017 Mattia Basaglia <mattia.basaglia@gmail.com>
