![image](./img/flameshot.png)
> Powerfull yet simple to use screenshot software.

**This program is in heavy develpment and it's incomplete!**

## Considerations
If you have a system-wide shortcut assigned to the`Print`
 key, you should disable it because it may interfere with the global key detection.

 In order to solve this problem I'm thinking about creating a cli tool to communicate with Flameshot via IPC socket; you could assign the specific command tto trigger the capture in a very convenient way. (that could be extended with many functions but it isn't a priority)

 Check the ./docs folder for more information.

## Dependencies
- QT (tested with QT5.8)
- QT X11
- X11 devel (with xcb)

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
