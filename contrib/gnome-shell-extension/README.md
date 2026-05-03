# Flameshot GNOME Shell integration

This optional extension lets Flameshot capture screenshots from inside GNOME
Shell on Wayland. GNOME denies normal applications direct access to
`org.gnome.Shell.Screenshot`, so Flameshot normally has to use
`xdg-desktop-portal`. On GNOME Wayland that portal path can produce a bright
flash.

When this extension is enabled, Flameshot first calls
`org.flameshot.ShellIntegration.Screenshot`. The extension forwards the request
to GNOME Shell's screenshot service with the Shell screenshot API's `flash`
argument set to `false`. It also returns the pointer position at capture time so
Flameshot can automatically choose the monitor under the pointer instead of
showing a monitor picker on multi-monitor GNOME Wayland sessions.

If the extension is unavailable or fails, Flameshot falls back to the existing
portal backend.

This extension currently targets GNOME Shell 49 and 50. It uses GNOME Shell
internals to add its own D-Bus service name to Shell's screenshot sender
allowlist. That avoids enabling `global.context.unsafe_mode` and prevents
GNOME's unsafe-mode warning notification.

## Install

Install for the current user:

```sh
contrib/gnome-shell-extension/install.sh
```

Or manually:

```sh
mkdir -p "${HOME}/.local/share/gnome-shell/extensions"
cp -r contrib/gnome-shell-extension/flameshot-native@flameshot.org \
  "${HOME}/.local/share/gnome-shell/extensions/"
gnome-extensions enable flameshot-native@flameshot.org
```

You may need to log out and back in before GNOME Shell sees a newly installed
extension.

Verify that GNOME Shell loaded the bridge:

```sh
gdbus introspect --session \
  --dest org.flameshot.ShellIntegration \
  --object-path /org/flameshot/ShellIntegration
```

The `Screenshot` method should expose `pointer_x` and `pointer_y` outputs:

```text
Screenshot(in s filename,
           out b success,
           out s filename_used,
           out i pointer_x,
           out i pointer_y)
```

Test the bridge directly:

```sh
gdbus call --session \
  --dest org.flameshot.ShellIntegration \
  --object-path /org/flameshot/ShellIntegration \
  --method org.flameshot.ShellIntegration.Screenshot \
  /tmp/flameshot-shell-bridge-test.png
```

## GNOME shortcut

To bind Flameshot to <kbd>Super</kbd> + <kbd>Shift</kbd> + <kbd>S</kbd>, first
clear GNOME Shell's built-in screenshot UI shortcut:

```sh
gsettings set org.gnome.shell.keybindings show-screenshot-ui "[]"
```

Then create a custom GNOME shortcut that runs:

```sh
flameshot gui
```

If Flameshot is installed into a non-standard prefix, use the full binary path,
for example:

```sh
${HOME}/.local/bin/flameshot gui
```
