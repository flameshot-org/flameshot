# Usage on minimal X11 window managers (i3, dwm, xmonad, ...)

Since v14, Flameshot captures the screen through the [XDG Desktop Portal](https://flatpak.github.io/xdg-desktop-portal/) (`org.freedesktop.portal.Desktop`), including on X11. Full desktop environments such as GNOME and KDE Plasma ship a portal backend that implements the `org.freedesktop.impl.portal.Screenshot` interface, so capturing works out of the box.

Minimal X11 window managers (i3, dwm, xmonad, bspwm, qtile, AwesomeWM, ...) usually do **not** provide such a backend: either no `xdg-desktop-portal` backend is running, or the backend that is installed (for example `xdg-desktop-portal-gtk`) does not implement the Screenshot interface. In that case the capture fails.

## Symptoms

When the portal service cannot be reached:

```text
flameshot: error: Could not locate the `org.freedesktop.portal.Desktop` service
flameshot: error: Unable to capture screen
flameshot: info: Screenshot aborted.
```

When a portal backend is present but does not implement the Screenshot interface (for example only `xdg-desktop-portal-gtk` is installed):

```text
flameshot: error: The xdg-desktop-portal backend did not respond If you are on
wayland make sure an xdg-desktop-portal backend for your desktop is installed
and properly configured.

If on X11 enable Legacy X11 method in the General Settings
```

In both cases the Flameshot capture overlay never appears.

## Fix: enable the legacy X11 capture

Flameshot ships a built-in option that bypasses the portal and uses Qt's native X11 capture (the method used before v14). Enable it once and capturing works again.

> [!NOTE]
> This option only affects X11 sessions; it is ignored on Wayland.

### From the GUI

Open **Configuration → General** and tick **"Use legacy X11 screenshot method"**.

### From the config file

Edit `~/.config/flameshot/flameshot.ini` and add, under the `[General]` section:

```ini
[General]
useX11LegacyScreenshot=true
```

Then restart the Flameshot tray daemon so the new setting is picked up:

```sh
killall flameshot 2>/dev/null; flameshot &
```

## Background: which portal backends implement Screenshot?

Not every `xdg-desktop-portal` backend implements `org.freedesktop.impl.portal.Screenshot`. Notably, the generic GTK backend (`xdg-desktop-portal-gtk`) does not, while the GNOME and KDE backends do but rely on their respective compositors (GNOME Shell / KWin), which are not running under a minimal window manager.

See the [list of backends and interfaces](https://wiki.archlinux.org/title/XDG_Desktop_Portal#List_of_backends_and_interfaces) on the Arch Wiki to check whether your installed backend supports the Screenshot interface.

## Affected window managers

The following X11 window managers have been reported to need the legacy option: i3, dwm, xmonad, bspwm, qtile, AwesomeWM, Xfce4, and Plasma on X11.

## Troubleshooting

**Q) I enabled the legacy option but the capture window is not full screen / behaves like a normal window.**

A) Some window managers tile or decorate the Flameshot window. Add a floating, borderless, fullscreen rule for the `flameshot` window class in your window manager configuration (similar to the rules documented for [Sway / wlroots](UsageHyprlandSwayWlroots.md)).
