# Smart region selection

### To be Reviewed By

* Flameshot maintainers
* macOS maintainers and users
* Contributors interested in equivalent Windows and Linux backends

### Authors

* Gábor Fekete and contributors

### Status: Draft

### Superseded by

N/A

### Related

* Issue #5
* Issue #1814
* Issue #2775

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

The preview dims everything outside the candidate, draws a colored outline, and
shows the candidate's role and position in the available ancestor chain. The
selection can still be resized and annotated using existing Flameshot tools.

The first backend targets macOS:

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

The detector is behind a platform-neutral interface so later Windows, X11, or
compositor-specific implementations can provide the same candidate contract.
Unsupported platforms receive no behavior change.

### Configuration and permissions

On macOS, a General setting named **Enable smart region selection** controls the
feature and is enabled by default. Window snapping works without Accessibility
permission. Selecting controls inside another application requires the standard
macOS Accessibility permission. General settings provides an explicit **Grant
Accessibility Access...** button that invokes the standard macOS permission
prompt before capture begins. Without that permission, Flameshot continues with
window-only detection.

## Performance Impact

Window enumeration occurs once per capture because Apple documents it as a
relatively expensive operation. Pointer hit-testing is throttled to at most once
every 35 milliseconds. Accessibility messaging uses a short timeout so an
unresponsive target application cannot indefinitely freeze the overlay. Painting
uses only the current candidate rectangle and label.

The candidate hierarchy is recalculated when the pointer moves. Scrolling only
changes an index in the already collected chain.

## Backwards Compatibility and Upgrade Path

Existing config files remain valid; the new Boolean setting has a default. The
manual drag gesture, committed selection, annotation tools, copy/save actions,
and command-line behavior are unchanged. Users who prefer the exact old initial
state can disable smart region selection in General settings.

The initial implementation is macOS-only. Other platforms compile a no-op
detector until they gain a backend.

## Prior Art and Alternatives

ShareX collects top-level and child-window rectangles before showing its region
overlay, ignores its own overlay window, highlights the first rectangle under
the pointer, and commits that rectangle into the normal selection workflow. The
proposed interaction follows that proven shape while replacing Win32 window
enumeration with native macOS Window Server and Accessibility APIs.

Computer-vision rectangle detection was discussed in #5 and #1814. It performs
poorly with borderless windows, shadows, overlapping dark surfaces, transparency,
and low contrast, and cannot expose a semantic parent hierarchy. Native metadata
is pixel-precise for normal windows and provides the element tree requested by
this interaction.

An active-window-only command is simpler but does not solve hover selection,
child elements, or the common case where the intended window is not focused.

## Drawbacks

* Element-level detection needs a macOS privacy permission.
* Accessibility quality depends on the target application's implementation.
  Some games, remote desktops, canvas-based interfaces, and custom controls may
  expose only a window or a coarse container.
* Platform-specific backends require separate testing and maintenance.
* Multi-display coordinate conversion and partially off-screen windows require
  an explicit test matrix, especially with mixed scale factors.

## Test Plan

* Window-only fallback with Accessibility denied.
* Leaf, parent, reverse-to-child, and containing-window navigation.
* Click-to-commit followed by resize, annotation, copy, and save.
* Drag-to-create a manual rectangle from inside a detected window.
* Chrome, Finder, native Qt, and a target that exposes few AX elements.
* Single Retina display, multiple equal-scale displays, mixed scaling, and a
  partially off-screen window.
* Accessibility target that is slow, closes during hit-testing, or refuses the
  requested attributes.
* Feature disabled and non-macOS builds.

## Errata

N/A
