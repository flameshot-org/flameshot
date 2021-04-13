# FAQ

## Flameshot freezes after capturing an image
By default Flameshot requires a notification manager. Notifications are sent to the notification manager via d-bus. If either d-bus or the notification manager is not properly configured, Flameshot will freeze for 30 seconds. 

Fix 1: Install a notification manager

Fix 2: Manually edit the config file at ~/.config/flameshot/flameshot.ini and add the following line: 
showDesktopNotification=false


## Flameshot doesn't start / no tray icon
When Flameshot is started with a system launcher or from the CLI it starts a daemon in the background. To interact with this daemon through a graphical client your desktop environment **must support a system tray**. See the README for tips on setting up a system tray.

This sometimes causes pain for Arch users running Gnome, because the system tray extension becomes out of sync with gnome upon a new gnome release. Please do not report this as a bug in Flameshot. We are reliant on a system tray and have no intention to change this.

## On MacOs Flameshot only captures a blank desktop
When running Flameshot on MacOs, you must grant it permission to screen record. 


## With fractional scaling my screen appears shifted
There is a known issue in Qt related to fractional scaling and full screen applications. We are hoping this is resolved in Qt6. See [this post](https://forum.qt.io/topic/121111/position-of-widget-with-fractional-scaling) for more details.