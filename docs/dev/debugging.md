# Debugging

## `FLAMESHOT_DEBUG_CAPTURE`

With this cmake variable set to `ON`, the flameshot capture GUI window won't bypass the
window manager. This allows you to manipulate the capture GUI window like any
other window while debugging.

This can be useful if a debugging breakpoint is triggered while flameshot is in
full screen mode. Without this variable, you might have trouble inspecting the
code due to a frozen full-screen window.

Usage:
```shell
cmake -DFLAMESHOT_DEBUG_CAPTURE=ON ...
```
