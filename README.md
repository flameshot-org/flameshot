![image](./img/flameshot.png)
> Powerfull yet simple to use screenshot software.

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

In case of doubt choose the first or the second shortcut in your favorite desktop environment.

A systray icon will be in your system's panel while Flameshot is running.
Do a right click on the tray icon and you'll see some menu items to open the configuration window and the information window.
Check out the information window to see all the available shortcuts in the graphical capture mode.

## Considerations

**Not working on Wayland**

- If you are using Gnome you need to install the [TopIcons](https://extensions.gnome.org/extension/495/topicons/) extension in order to see the systemtray icon.

- In order to speed up the first launch of Flameshot (DBus init of the app can be slow), consider starting an application automatically on boot.

- Press `Enter` or `Ctrl + C` when you are in a capture mode and you don't have an active selection and the whole desktop will be copied to your clipboard!

## Compilation and development

- Information about manual compilation can be found [here](./docs/dev/compilation.md)

- Check the [docs](./docs) folder for more information.

## Screenshots
Dynamic button position based on your selection!

![image](./img/appScreenshots/screenshot_1.png)

Choose your buttons and edit your screenshots!
![image](./img/appScreenshots/screenshot_2.png)

## License
- The main code is licensed under GPLv3
- The logo of Flameshot is licensed under Free Art License v1.3
- The button icons are licensed under Apache License 2.0. See: https://github.com/google/material-design-icons
- The code at capture/capturewidget.cpp is based on https://github.com/ckaiser/Lightscreen/blob/master/dialogs/areadialog.cpp (LGPL/GPL)
- The code at capture/capturewidget.h is based on https://github.com/ckaiser/Lightscreen/blob/master/dialogs/areadialog.h (LGPL/GPL)
- Qt-Color-Widgets taken and modified from https://github.com/mbasaglia/Qt-Color-Widgets (see their license and exceptions in the project) (LGPL/GPL)

## Acknowledgment
I really appreciate those who have shown interest in the develpment process:
- Cosmo.
- ismatori.
- The members of the Sugus GNU/Linux association.
