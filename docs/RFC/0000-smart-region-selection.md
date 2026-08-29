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

The detector is behind a platform-neutral interface. Every backend returns the
same leaf-to-window candidate chain in global logical coordinates; the capture
widget owns preview, navigation, and selection behavior. A backend can report
that it is unavailable, in which case Flameshot retains the current manual
selection behavior without installing any new event handling.

The intended platform rollout is:

* macOS: Window Server geometry plus Accessibility hit-testing, as described
  above;
* Windows: top-level window enumeration plus UI Automation hit-testing and
  parent traversal;
* X11: EWMH stacking and window geometry, with AT-SPI providing an optional
  child-element hierarchy;
* Wayland: a compositor-specific backend only when its APIs can provide the
  same contract. The current screenshot portals do not provide arbitrary
  foreign window and control geometry, so the generic backend remains disabled
  rather than approximating or changing the manual workflow.

The macOS backend is proposed first because it provides a working reference for
the shared contract. Acceptance of this RFC should decide whether staged native
backends are acceptable or whether multiple backends must be delivered in the
first implementation pull request.

### Configuration and permissions

On macOS, a General setting named **Enable smart region selection** controls the
feature and is disabled by default during the staged rollout. Enabling it is an
explicit choice because preselection clicks and scrolling gain new behavior.
Window snapping works without Accessibility permission. Selecting controls
inside another application requires the standard macOS Accessibility
permission. General settings provides an explicit **Grant Accessibility
Access...** button that invokes the standard macOS permission prompt before
capture begins. Without that permission, Flameshot continues with window-only
detection.

## Performance Impact

Window enumeration occurs once per capture because Apple documents it as a
relatively expensive operation. Pointer hit-testing is throttled to at most once
every 35 milliseconds. Accessibility messaging uses a short timeout so an
unresponsive target application cannot indefinitely freeze the overlay. Painting
uses only the current candidate rectangle and label.

The candidate hierarchy is recalculated when the pointer moves. Scrolling only
changes an index in the already collected chain.

## Backwards Compatibility and Upgrade Path

Existing config files remain valid; the new Boolean setting defaults to false.
The manual drag gesture, committed selection, annotation tools, copy/save
actions, and command-line behavior are unchanged. Unsupported platforms and
users who do not opt in receive no behavior change.

The initial implementation is macOS-only. Other platforms compile a no-op
detector until they gain a backend.

## Prior Art and Alternatives

ShareX collects top-level and child-window rectangles before showing its region
overlay, ignores its own overlay window, highlights the first rectangle under
the pointer, and commits that rectangle into the normal selection workflow. The
proposed interaction follows that proven shape while replacing Win32 window
enumeration with native macOS Window Server and Accessibility APIs.

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
* Accessibility quality depends on the target application's implementation.
  Some games, remote desktops, canvas-based interfaces, and custom controls may
  expose only a window or a coarse container.
* Platform-specific backends require separate testing and maintenance.
* A staged rollout temporarily gives supported platforms more capability than
  unsupported platforms, even though existing behavior remains unchanged.
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
