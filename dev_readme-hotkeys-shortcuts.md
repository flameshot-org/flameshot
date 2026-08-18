# Hotkeys & Shortcuts

## Incident: Ctrl+E (and shortcuts in general) randomly not firing

**Symptom:** after opening a capture (Print), keyboard shortcuts like
Ctrl+E (toggle side panel) would work sometimes and silently do nothing
other times. Also present: a "Flameshot Error / The configuration
contains an error" notification flashing on almost every single launch,
immediately followed by "successfully resolved."

**What it looked like it was:** a window manager focus problem. Under
xfwm4, a newly-opened capture window doesn't always become the genuine
X11-focused window even after `activateWindow()`/`raise()` - confirmed
directly by comparing `xdotool getactivewindow` against the window
Flameshot had just opened. Since Qt's `QShortcut` only fires when its
window is the active one, this looked like a completely sufficient
explanation, and several fixes were built around it:

1. A delayed re-activation retry after opening
2. Swapping the window's `Qt::Tool` flag for `Qt::Window` (this genuinely
   *did* fix real X11 focus, confirmed via `xdotool getwindowfocus`) -
   but it made window stacking/rendering intermittently unstable
   (occasional crash, occasional render-behind-other-windows), a worse
   problem than the one it solved. Reverted.
3. Changing the `QShortcut` context from `Qt::WindowShortcut` to
   `Qt::ApplicationShortcut`
4. A raw `XSetInputFocus` call bypassing the window manager entirely,
   isolated into its own translation unit to avoid Xlib/Qt macro
   collisions (`Bool`, `True`, `None`, etc. from `<X11/Xlib.h>` break Qt
   headers if they ever share a compile unit) - this also caused crashes
   (likely from opening ad-hoc Xlib connections alongside Qt's own xcb
   connection to the same X server)
5. Firing the toggle over D-Bus from an OS-level global hotkey instead of
   a Qt shortcut at all (sidesteps window focus completely) - the D-Bus
   call itself worked, but exposed that a *separate* `flameshot gui` CLI
   process's capture window is not the same object as the daemon's
   `m_captureWindow`, so the daemon-side D-Bus call had nothing to act on

None of these fully fixed it, and #2 and #4 actively made things worse.
All were reverted.

**What it actually was:** `TYPE_SAVE_LOCATION_1/2/3` (added for the
per-location save hotkeys) had no entry in `confighandler.cpp`'s
`SHORTCUT(...)` table (see the SOP below - this is step 4, and it's the
step that's easy to forget). The moment a key got bound to one of those
actions, `~/.config/flameshot/flameshot.ini` gained a line like
`TYPE_SAVE_LOCATION_1=Ctrl+D` that the config validator didn't recognize.
`resolveAnyConfigErrors()` is called at the *start* of every single
`Flameshot::gui()` invocation and, on failure, calls
`ConfigResolver::exec()` - a **blocking modal dialog** - before the
capture window is shown. That dialog opening and auto-resolving on every
single launch was disrupting Qt's focus/activation state at exactly the
moment the capture window needed to establish itself as active, which is
what made keyboard shortcuts fire inconsistently afterward.

**The actual fix:** add the three missing entries to the `SHORTCUT(...)`
table in `confighandler.cpp` (see step 4 below). One line each. Once the
config stopped being flagged as invalid, the blocking resolver dialog
stopped firing on every launch, and shortcuts (including Ctrl+E) started
working reliably.

**Lesson:** a flaky, hard-to-reproduce input/focus bug is worth checking
against "is something *else* stealing the event loop or focus at the
same moment" before assuming it's a deep window-manager limitation. Five
targeted fixes for the assumed root cause (WM focus) were built, tested,
and mostly reverted before the actual cause (a blocking dialog from an
unrelated validation gap) was found. The retry/timer-based
`activateWindow()` calls from attempts #1 and #3's era were left in
place since they're harmless and can only help; the code for #2, #4, and
#5 was fully reverted.

---

## SOP: Adding a new capture-tool hotkey

How to add a new action to Flameshot's capture editor that shows up in
the Shortcuts tab (Configuration > Shortcuts) with a user-rebindable key,
using `SaveLocationTool` (`src/tools/savelocation/`) as the worked
example.

There are **5 required steps**. Missing step 4 is the exact trap
described in the incident above: the app still builds and runs fine, but
the moment a user binds a key to your new action (or Flameshot itself
persists the entry), it gets written to
`~/.config/flameshot/flameshot.ini` under `[Shortcuts]`, and on the next
launch the config validator doesn't recognize the key and throws up a
blocking "Configuration file has errors" dialog - which, per the incident
above, can also make keyboard shortcuts flaky app-wide, not just show an
error. It doesn't show up until someone actually binds the key, so it's
easy to miss in your own testing if you don't try binding it yourself.

### 1. Add the enum value

`src/tools/capturetool.h`, inside `CaptureTool::Type`. Append at the
**bottom** - the comment there is not decorative, existing users'
persisted ini files reference these by their integer value:

```cpp
TYPE_SAVE_LOCATION_1 = 25,
TYPE_SAVE_LOCATION_2 = 26,
TYPE_SAVE_LOCATION_3 = 27,
```

### 2. Implement the tool

A `CaptureTool` subclass (see `src/tools/savelocation/savelocationtool.h/.cpp`
for a full example, or copy `src/tools/save/savetool.*` for the simplest
possible one). Minimum required overrides: `type()`, `name()`,
`description()` (this is the text shown in the Shortcuts tab's
Description column - keep it short, it's a table cell, not a sentence),
`icon()`, `copy()`, `closeOnButtonPressed()`, and `pressed()` (the actual
behavior).

Register the new .cpp/.h in the relevant `CMakeLists.txt`
(`src/tools/CMakeLists.txt` for tools) via `target_sources(flameshot
PRIVATE ...)`. New files aren't picked up automatically - if you add a
file without this, you'll get a linker error ("undefined reference") on
the *next* full reconfigure+build, but a *plain* rebuild will silently
skip it, which is confusing if you don't know to expect it. Always run
`cmake -S . -B build ...` again after adding a new source file, before
`cmake --build build`.

### 3. Register it in ToolFactory

`src/tools/toolfactory.cpp` - add a case to `ToolFactory::CreateTool()`.
If your constructor takes extra args beyond `parent` (like
`SaveLocationTool(int location, QObject* parent)` does), you can't use
the `if_TYPE_return_TOOL` macro; write the `case` by hand.

### 4. Register the default shortcut - THE STEP THAT'S EASY TO FORGET

`src/utils/confighandler.cpp`, in the `SHORTCUT(...)` table (search for
`SHORTCUT("TYPE_PIN"` to find it). Add an entry even if the default key
is empty:

```cpp
SHORTCUT("TYPE_SAVE_LOCATION_1"     ,                           ),
```

This is what makes `checkUnrecognizedSettings()` accept the key once it's
persisted to the ini. Without it, the config validator treats any ini
line like `TYPE_SAVE_LOCATION_1=Ctrl+D` as garbage and blocks the app on
every launch with "Configuration file has errors" until the user manually
removes the offending line via the resolver dialog - see the incident
above for how disruptive this actually is.

If you want the action pre-bound to a specific key out of the box (not
just present-but-unbound), that's also done here:

```cpp
SHORTCUT("TYPE_IMAGEUPLOADER"       ,   "Ctrl+D"                ),
```

### 5. Add it to the Shortcuts-tab / toolbar list

`src/widgets/capture/capturetoolbutton.cpp`,
`CaptureToolButton::iterableButtonTypes` - this exact array (in this exact
order) is what the Shortcuts tab iterates to build its row list, so
position in this array = row position in that table. Add to
`buttonTypeOrder` too if you also want a specific position among the
capture-editor's on-screen toolbar icons (a separate, cosmetic concern -
`iterableButtonTypes` alone is enough for the Shortcuts tab to work).

### After all 5 steps

```bash
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/opt/flameshot -DCMAKE_BUILD_TYPE=Release -DENABLE_IMGUR=ON
cmake --build build --parallel "$(nproc)"
sudo cmake --install build
```

Then restart the daemon (`pkill flameshot && flameshot &`) and actually
**bind a key to the new action in Configuration > Shortcuts and confirm no
error dialog appears on the next launch** - this is the step that catches
a forgotten step 4, since everything else works fine right up until a key
actually gets bound.
