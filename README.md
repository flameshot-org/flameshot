![image](./img/flameshot.png)
> Powerfull yet simple to use screenshot software.

**A beta will be released and packaged for the main GNU/Linux distros soon !**

## Usage

When you execute the binary, the app will create a systray icon, the app will be waiting for the Print Key to start a new capture.

Do a right click on the tray icon and you'll see menu items to open the configuration window and the information window.
Check out the information window to see all the available shortcuts in the capture mode.

This software is very focused on selection capture and edition, but you can do full screen captures very easily just not having a visible selection, by default it will take it as if you were selecting the whole screen.

## Considerations

**Not working on Wayland**

If you have a system-wide shortcut assigned to the`Print`
 key, you should disable it because it may interfere with the global key detection.

 In order to solve this problem I'm thinking about creating a cli tool to communicate with Flameshot via IPC socket; you could assign the specific command tto trigger the capture in a very convenient way. (that could be extended with many functions but it isn't a priority)

 Check the ./docs folder for more information.

## Dependencies
- QT (tested with QT5.8)
- QT X11 extras
- X11 devel (with xcb)

## Compilation
Just download QT Creator and the dependencies (see the previous section) and import the project selecting the `flameshot.pro` file. Hit the "Build" button.

## Screenshots
Dynamic button position based on your selection!

![image](./img/appScreenshots/screenshot_1.png)

Choose your buttons and edit your screenshots!
![image](./img/appScreenshots/screenshot_2.png)

## License
- The main code is licensed under GPLv3
- The logo of Flameshot is licensed under Free Art License v1.3
- The button icons are licensed under Apache License 2.0. See: https://github.com/google/material-design-icons
- The code at capture/capturewidget.cpp is based on https://github.com/ckaiser/Lightscreen/blob/master/dialogs/areadialog.cpp
- The code at capture/capturewidget.h is based on https://github.com/ckaiser/Lightscreen/blob/master/dialogs/areadialog.h
- Qt-Color-Widgets taken and modified from https://github.com/mbasaglia/Qt-Color-Widgets (see their license and exceptions in the project)

## Acknowledgment
I really appreciate those who have shown interest in the develpment process:
- Cosmo.
- ismatori.
- The members of the Sugus association.
