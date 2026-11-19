# Smart region selection

### To be Reviewed By

* Flameshot maintainers
* macOS, Windows and Linux (X11) maintainers and users

### Authors

* Gábor Fekete and contributors

### Status: Draft

### Superseded by

N/A

### Related

* Issue #5 (canonical window-selection request)
* Issue #4910 (ShareX-style macOS feasibility patch)
* Issue #4239 (window and panel selection)
* Issue #1814 (rectangle recognition)

## Problem

Flameshot's GUI capture mode starts with no selected area. Capturing a whole
window, dialog, toolbar, or other rectangular interface element therefore
requires carefully drawing its bounds by hand. This is slower and less precise
than region capture in tools such as ShareX, which detects the window or control
under the pointer.

The existing requests cover selecting a whole window, but users also benefit
from selecting a meaningful child element such as a panel, table, or dialog.
Flameshot must retain its current freehand rectangle workflow and its ability to
annotate the chosen region before copying or saving it.

## Anti-Goals

* This RFC does not add automatic capture or bypass the existing annotation UI.
* It does not require every platform backend to ship at the same time.
* It does not use computer vision to infer borders from screenshot pixels.
* It does not make inaccessible or hidden UI elements visible.
* It does not change Wayland security or compositor protocols.

## Solution

Add an optional smart region detector to GUI capture. Before the capture overlay
appears, the detector records the visible top-level window stack. While no area
is selected, moving the pointer over a detected region previews that region:

* a click commits the preview as the normal editable Flameshot selection;
* a drag keeps the existing custom rectangle behavior;
* scrolling up walks from the deepest detected element toward its parent and
  ultimately the containing window;
* scrolling down walks back toward the previously detected child;
* after an area is committed, the mouse wheel retains its current tool-size
  behavior.

The preview dims everything outside the candidate and draws a colored outline.
It deliberately avoids an inline role/index label because that label can cover
the content the user is trying to inspect. The selection can still be resized
and annotated using existing Flameshot tools.

The macOS backend uses native window and Accessibility APIs:

1. `CGWindowListCopyWindowInfo` records normal, on-screen windows in front-to-
   back order before Flameshot's overlay exists.
2. The window under the global pointer identifies the owning process.
3. `AXUIElementCopyElementAtPosition` is called with that application's
   accessibility object, so hit-testing remains restricted to the underlying
   application even though the Flameshot overlay is now topmost.
4. `kAXParentAttribute`, `kAXPositionAttribute`, and `kAXSizeAttribute` produce
   the selectable leaf-to-window chain.
5. Duplicate, tiny, invalid, and off-window rectangles are removed. The Window
   Server geometry remains a fallback when Accessibility access is unavailable.

The Windows backend follows the same contract:

1. `EnumWindows` records visible top-level windows before the overlay appears.
2. `DwmGetWindowAttribute` supplies extended frame bounds and filters cloaked
   windows, with `GetWindowRect` as a fallback.
3. MSAA hit-testing (`accHitTest`) asks the application, inside the innermost
   child window under the pointer, which element is at the position, one call
   per level, and `accParent` provides the containing elements. This stays
   fast in large trees such as web pages. When an application only reports
   the window, Microsoft UI Automation is searched instead: each level of
   control-view children is fetched with its bounding rectangles in one
   cached `FindAll` call.
4. Lookups run on a dedicated worker thread, and UI Automation calls have
   connection and transaction timeouts. The GUI thread waits at most 60
   milliseconds and shows the window meanwhile; a slower lookup keeps running
   for up to 1.5 seconds, and its result is picked up by the follow-up lookups
   while the pointer rests. Closing the overlay does not wait for an
   application that hangs.
5. Native screen coordinates are mapped to Qt logical coordinates per monitor,
   including mixed-scale displays.
6. Chromium-based applications (Chrome, Edge, Electron) only expose their
   web content after an assistive tool hit-tests them through UI Automation.
   That hit-test would find the capture overlay, so when the windows are
   recorded, before the overlay appears, the backend hit-tests the pointer
   and the visible Chromium windows, waiting at most 200 milliseconds. The
   applications build the tree in about a second, and the follow-up lookups
   then find the elements.

The X11 backend follows the same contract:

1. `_NET_CLIENT_LIST_STACKING` on the root window lists the managed windows in
   stacking order. Unmapped, minimized (`_NET_WM_STATE_HIDDEN`) and desktop
   windows are skipped. `_NET_FRAME_EXTENTS` adds window manager decorations,
   and `_GTK_FRAME_EXTENTS` removes the invisible shadows of client-side
   decorations. All requests are pipelined in a single round trip.
2. `_NET_WM_PID` identifies the owning process, which is matched to the
   application registered on the AT-SPI accessibility bus.
3. AT-SPI `Component.GetAccessibleAtPoint` descends from the application's
   top-level element that matches the window to the deepest element under the
   pointer; `Component.GetExtents` provides each element's geometry.
4. D-Bus calls share the same per-lookup budget and per-call timeout as macOS.
   Without an accessibility bus, the backend detects whole windows only.
5. The backend is only enabled when Qt uses the `xcb` platform in an X11
   session. It is built when the `xcb` development files are available.

The detector is behind a platform-neutral interface. Every backend returns the
same leaf-to-window region chain in global logical coordinates. A small
controller owns the highlighted region, wheel navigation and lookup rate
limiting; the capture widget only forwards pointer and wheel input and paints
the controller's current region. Supported backends fall back to window
geometry when Accessibility, UI Automation or AT-SPI is unavailable.
Unsupported platforms and sessions do not show the setting and keep manual
selection only.

The highlighted region is exactly what a click selects. Some applications
(Chromium, Electron, WebKit) build their accessibility tree lazily, so the
first lookup can only return a coarse container. Instead of silently replacing
the highlighted region when the pointer is pressed, the controller repeats the
lookup a few times while the pointer rests and updates the highlight, so it
settles on the deepest element before the user clicks. Lookups for an
unchanged pointer position are ignored. On macOS, Electron applications are
asked to expose their accessibility tree with the documented
`AXManualAccessibility` attribute.

The platform support is:

* macOS: Window Server geometry plus Accessibility hit-testing;
* Windows: top-level window enumeration plus UI Automation hit-testing and
  parent traversal;
* X11: EWMH window stacking and geometry plus AT-SPI hit-testing;
* Wayland: unsupported. The screenshot portals do not expose foreign window or
  control geometry, so the setting is hidden and manual selection is unchanged.

### Configuration and permissions

On supported platforms, a General setting named **Enable smart region
selection** controls the feature and is disabled by default. Enabling it is an explicit
choice because preselection clicks and scrolling gain new behavior. On macOS,
window snapping works without Accessibility permission, while selecting
controls inside another application requires the standard macOS Accessibility
permission. General settings provides an explicit **Grant Accessibility
Access...** button on macOS only. Windows UI Automation and AT-SPI do not
require a permission prompt.

## Performance Impact

Window enumeration occurs once per capture. Pointer hit-testing is throttled to
at most once every 35 milliseconds, and up to five follow-up lookups run while
the pointer rests (after 150, 400, 1000, 2000 and 3500 milliseconds). Accessibility and AT-SPI
calls use 80-millisecond limits within a 200-millisecond request budget on the
GUI thread. UI Automation runs on a worker thread with 300-millisecond call
limits and a 1.5-second budget; the GUI thread waits at most 60 milliseconds. These
are best-effort limits, not process isolation or a guarantee against arbitrary
provider stalls or COM teardown delays. Painting uses only the current region.

The region hierarchy is recalculated when the pointer moves. Scrolling only
changes an index in the already collected chain.

## Backwards Compatibility and Upgrade Path

Existing config files remain valid; the new Boolean setting defaults to false.
The manual drag gesture, committed selection, annotation tools, copy/save
actions, and existing command-line options remain available. Smart-region click
and wheel behavior applies only when enabled on a supported platform.

Platforms without a backend compile no detector code and do not show the
setting.

## Prior Art and Alternatives

ShareX collects top-level and child-window rectangles before showing its region
overlay, ignores its own overlay window, highlights the first rectangle under
the pointer, and commits that rectangle into the normal selection workflow. The
proposed interaction follows that proven shape using native macOS Window Server
and Accessibility APIs, or Windows window geometry and UI Automation.

Issue #4910 includes an earlier working macOS patch that demonstrates Window
Server and Accessibility-based hover snapping. This RFC retains that native
approach but proposes a platform-neutral detector boundary, a complete
leaf-to-window candidate chain, explicit parent/child navigation, throttled
hit-testing, an explicit permission request, and an opt-in setting. The patch is
useful prior art and evidence of feasibility; it does not yet address the shared
backend contract requested by maintainers.

Computer-vision rectangle detection was discussed in #5 and #1814. It performs
poorly with borderless windows, shadows, overlapping dark surfaces, transparency,
and low contrast, and cannot expose a semantic parent hierarchy. Native metadata
is pixel-precise for normal windows and provides the element tree requested by
this interaction.

An active-window-only command is simpler but does not solve hover selection,
child elements, or the common case where the intended window is not focused.

## Drawbacks

* Element-level detection needs a macOS privacy permission.
* Accessibility and UI Automation quality depend on the target application's
  implementation. Some games, remote desktops, canvas-based interfaces, and
  custom controls may expose only a window or a coarse container.
* On Linux, Qt, Chromium, Electron and Firefox applications only expose
  AT-SPI when desktop accessibility is enabled; otherwise only GTK
  applications provide elements and the rest fall back to whole windows.
  Flatpak and Snap sandboxes restrict access to the accessibility bus.
* Platform-specific backends require separate testing and maintenance.
* A staged rollout temporarily gives supported platforms more capability than
  unsupported platforms, even though existing behavior remains unchanged.
* Multi-display coordinate conversion and partially off-screen windows require
  an explicit test matrix, especially with mixed scale factors.

## Test Plan

* Window-only fallback with Accessibility denied.
* Leaf, parent, reverse-to-child, and containing-window navigation.
* Physical upward scrolling moves toward the parent with both natural and
  traditional scrolling enabled; downward scrolling moves toward the child.
  On Windows, Qt does not report natural touchpad scrolling, and touchpad
  scrolling arrives as ordinary wheel messages. Raw input from the precision
  touchpad, which arrives while fingers are on it, identifies its wheel
  messages, and the touchpad's `ScrollDirection` setting is applied to them.
* Before a selection is committed, every wheel event is consumed by smart-region
  navigation and never changes the annotation tool size.
* Click-to-commit followed by resize, annotation, copy, and save.
* Drag-to-create a manual rectangle from inside a detected window.
* Chrome, Finder or File Explorer, native Qt, and a target that exposes few
  accessibility elements.
* Windows App and RDP content, which may expose only a coarse remote surface.
* Single Retina display, multiple equal-scale displays, mixed scaling, and a
  partially off-screen window.
* X11 with a reparenting window manager, a client-side decorated (GTK) window,
  a Qt application, and a session without the AT-SPI bus. Wayland sessions do
  not show the setting.
* Hovering a Chromium or Electron window right after starting a capture: the
  highlight settles on the deepest element, and clicking without moving the
  pointer selects the highlighted region.
* Accessibility target that is slow, closes during hit-testing, or refuses the
  requested attributes.
* Feature disabled and unsupported-platform builds.

## Errata

N/A
