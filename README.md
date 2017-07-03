# Flameshot
![image](./img/flameshot.png) 
> Powerful yet simple to use screenshot software.

## Usage Preview
![image](./img/appPreview/animatedUsage.gif)

## Index
- [Features](#features)
- [Usage](#usage)
- [Shortcuts](#shortcuts)
- [Considerations](#considerations)
- [Compilation](#compilation)
  - [Debian](#debian)
  - [Fedora](#fedora)
  - [Arch](#arch)
  - [Install](#install)
- [License](#license)

## Features
- Customizable appearance.
- Easy to use.
- In-app screenshot edition.
- DBus interface.
- Upload to Imgur.

## Usage
Example commands:
- capture with GUI:

`flameshot gui`
- capture with GUI with custom save path:

`flameshot gui -p ~/myStuff/captures`
- fullscreen capture (asking savepath):

`flameshot full`
- fullscreen capture with custom save path (no GUI):

`flameshot full -p ~/myStuff/captures`
- fullscreen capture with custom save path copying to clipboard:

`flameshot full -c -p ~/myStuff/captures`

In case of doubt choose the first or the second command as shortcut in your favorite desktop environment.

A systray icon will be in your system's panel while Flameshot is running.
Do a right click on the tray icon and you'll see some menu items to open the configuration window and the information window.
Check out the information window to see all the available shortcuts in the graphical capture mode.

## Shortcuts

These shortcuts are available in GUI mode:

|  Keys         |  Description                |
|---            |---                          |
|  ←↓↑→         | Move selection 1px          |
| SHIFT + ←↓↑→  | Resize selection 1px        |
| ESC           | Quit capture                |
| CTRL + C      | Copy to clipboard           |
| CTRL + S      | Save selection as a file    |
| CTRL + Z      | Undo the last modification  |
| Right Click   | Show color picker           |

## Considerations

- **Not working on Wayland**

- If you are using Gnome you need to install the [TopIcons](https://extensions.gnome.org/extension/495/topicons/) extension in order to see the systemtray icon.

- In order to speed up the first launch of Flameshot (DBus init of the app can be slow), consider starting the application automatically on boot.

- Press `Enter` or `Ctrl + C` when you are in a capture mode and you don't have an active selection and the whole desktop will be copied to your clipboard! Pressing `Ctrl + S` will save your capture in a file! Check the [Shortcuts](#shortcuts) for more information.

- Execute `flameshot` without parameters to launch a running instance of the program without taking actions.

## Compilation
### Debian
Compilation Dependencies:
````
apt install -y git g++ build-essential qt5-qmake qt5-default
````

Compilation: run `qmake && make` in the main directory.

Runtime Dependencies (not needed if compiled from source):
````
apt install -y libqt5dbus5, libqt5network5, libqt5core5a, libqt5widgets5, libqt5gui5
````

### Fedora
Compilation Dependencies:
````
dnf install -y qt5-devel gcc-c++ git qt5-qtbase-devel
````

Compilation:  run `qmake-qt5 && make` in the main directory.

Runtime Dependencies (not needed if compiled from source):
````
dnf install -y qt5-qtbase
````

### Arch
Compilation Dependencies:
````
pacman -S git qt5-base base-devel
````

Compilation:  run `qmake && make` in the main directory.

Runtime Dependencies (not needed if compiled from source):
````
pacman -S qt5-base
````

## Install

Simply use `make install` with privileges.

## License
- The main code is licensed under [GPLv3](./LICENSE)
- The logo of Flameshot is licensed under [Free Art License v1.3](./img/flameshotLogoLicense.txt)
- The button icons are licensed under Apache License 2.0. See: https://github.com/google/material-design-icons
- The code at capture/capturewidget.cpp is based on https://github.com/ckaiser/Lightscreen/blob/master/dialogs/areadialog.cpp (LGPL/GPL)
- The code at capture/capturewidget.h is based on https://github.com/ckaiser/Lightscreen/blob/master/dialogs/areadialog.h (LGPL/GPL)
- Qt-Color-Widgets taken and modified from https://github.com/mbasaglia/Qt-Color-Widgets (see their license and exceptions in the project) (LGPL/GPL)

## Acknowledgment
I really appreciate those who have shown interest in the develpment process:
- Cosmo.
- ismatori.
- The members of the Sugus GNU/Linux association.
