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
- [Installation](#installation)
- [Compilation](#compilation)
  - [Debian](#debian)
  - [Fedora](#fedora)
  - [Arch](#arch)
  - [Install](#install)
- [Packaging](#packaging)
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

- open GUI with a delay of 2 seconds:

`flameshot gui -d 2000`

- fullscreen capture (asking savepath):

`flameshot full`

- fullscreen capture with custom save path (no GUI) and delayed:

`flameshot full -p ~/myStuff/captures -d 5000`

- fullscreen capture with custom save path copying to clipboard:

`flameshot full -c -p ~/myStuff/captures`

In case of doubt choose the first or the second command as shortcut in your favorite desktop environment.

A systray icon will be in your system's panel while Flameshot is running.
Do a right click on the tray icon and you'll see some menu items to open the configuration window and the information window.
Check out the information window to see all the available shortcuts in the graphical capture mode.

### CLI configuration
You can use the graphical menu to configure Flameshot, but alternatively you can use your terminal or scripts to do so.

- open the confguration menu:

`flameshot config`

- show the initial help message in the capture mode:

`flameshot config --showhelp true`

- for more information about the available options use the help flag:

`flameshot config -h`

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
| Mouse Wheel   | Change the tool's thickness |

Shift + drag a handler of the selection area: mirror redimension in the opposite handler.

## Considerations

- Experimental Gnome Wayland and Plasma Wayland support.

- If you are using Gnome you need to install the [TopIcons](https://extensions.gnome.org/extension/1031/topicons/) extension in order to see the systemtray icon.

- In order to speed up the first launch of Flameshot (DBus init of the app can be slow), consider starting the application automatically on boot.

- Press `Enter` or `Ctrl + C` when you are in a capture mode and you don't have an active selection and the whole desktop will be copied to your clipboard! Pressing `Ctrl + S` will save your capture in a file! Check the [Shortcuts](#shortcuts) for more information.

- Execute the command `flameshot` without parameters or use the "Launch Flameshot" desktop entry to launch a running instance of the program without taking actions.

## Installation

There are a packages available for a few distros:
- [Arch](https://aur.archlinux.org/packages/flameshot/)
- [Open Suse](https://software.opensuse.org/package/flameshot)
- [Void Linux](https://github.com/voidlinux/void-packages/tree/master/srcpkgs/flameshot) (`xbps-install flameshot`)

If you are not using any of these distros you'll need to compile the program :(
but don't worry, it's pretty easy!

## Compilation
The compilation requires Qt version 5.3 or higher (this is the version that Debian 8 has in its repos, so most modern distros should be able to compile without installing newer Qt versions).

### Debian
Compilation Dependencies:
````
apt install -y git g++ build-essential qt5-qmake qt5-default qttools5-dev-tools
````

Compilation: run `qmake && make` in the main directory.

### Fedora
Compilation Dependencies:
````
dnf install -y qt5-devel gcc-c++ git qt5-qtbase-devel qt5-linguist
````

Compilation:  run `qmake-qt5 && make` in the main directory.

### Arch
Compilation Dependencies:
````
pacman -S git qt5-base base-devel qt5-tools
````

Compilation:  run `qmake && make` in the main directory.

### Install

Simply use `make install` with privileges.

## Packaging

In order to generate the makefile installing in `/usr` instead of in `/usr/local` you can use the `packaging` option to generate the proper makefile (`qmake CONFIG+=packaging` instead of just `qmake`).

If you want to install in a custom directory you can define the `BASEDIR` variable.

**Example**:
You whant to install Flameshot in ~/myBuilds/test. You would execute the following to do so:
`qmake CONFIG+=packaging BASEDIR=~/myBuilds/test && make install`

### Runtime Dependencies

**Debian**:
````
libqt5dbus5, libqt5network5, libqt5core5a, libqt5widgets5, libqt5gui5
````
Optional:
```
openssl, ca-certificates
```

**Fedora**:
````
qt5-qtbase
````
Optional:
```
openssl, ca-certificates
```

**Arch**:
````
qt5-base
````
Optional:
```
openssl, ca-certificates
```

## License
- The main code is licensed under [GPLv3](./LICENSE)
- The logo of Flameshot is licensed under [Free Art License v1.3](./img/flameshotLogoLicense.txt)
- The button icons are licensed under Apache License 2.0. See: https://github.com/google/material-design-icons
- The code at capture/capturewidget.cpp is based on https://github.com/ckaiser/Lightscreen/blob/master/dialogs/areadialog.cpp (GPLv2)
- The code at capture/capturewidget.h is based on https://github.com/ckaiser/Lightscreen/blob/master/dialogs/areadialog.h (GPLv2)
- I copied a few lines of code from KSnapshot regiongrabber.cpp revision 796531 (LGPL)
- Qt-Color-Widgets taken and modified from https://github.com/mbasaglia/Qt-Color-Widgets (see their license and exceptions in the project) (LGPL/GPL)

Info: If I take code from your project and that implies a relicense to GPLv3, you can reuse my changes with the original previous license of your project applied.

## Acknowledgment
I really appreciate those who have shown interest in the develpment process:
- Cosmo.
- ismatori.
- The members of the Sugus GNU/Linux association.
