# Windows: multi-monitor capture and overlay alignment

This note summarizes **what was wrong** on Windows with heterogeneous multi-monitor layouts and **what to change** so capture, compositing, and cropping stay consistent with the virtual desktop.

## Symptom

On Windows 11 with multiple displays (e.g. negative `X`, different heights, non-zero secondary `Y`), users could see:

- black regions in the composite screenshot;
- selection / overlay offset from the real image;
- cropped or "swapped" monitor content.

The behaviour often changed when switching which display was primary.

## Root cause

`ScreenGrabber::windowsScreenshot()` placed each per-screen `grabWindow()` pixmap on a canvas using **only**:

- summed widths of monitors fully to the **left** of the current screen, and
- summed heights of monitors fully **above** the current screen.

That approximates a **striped grid**, not the true **2D virtual desktop** layout. It breaks when monitors are staggered (e.g. left monitor has a different `Y` than the right one): the composite buffer no longer matches the union of `QScreen::geometry()` in pixel space.

`cropToMonitor()` on Windows reused the same "sum left / sum above" logic, so the crop rectangle did not match the composite either.

## Intended behaviour (reference)

Treat the desktop as one rectangle: the union of all screen geometries in **logical** coordinates, with origin at the **minimum** `(x, y)`. Each monitor's pixmap is pasted at:

```text
physicalOffset = (screen.geometry().topLeft() - virtualTopLeft) * screen.devicePixelRatio()
```

(using each screen's own DPR for that monitor's offset and size). This matches the idea of capturing **`SystemInformation.VirtualScreen`** in one coherent coordinate system (e.g. ShareX / `BitBlt` over the virtual rectangle).

## Code changes (minimal)

1. **`src/utils/screengrabber.cpp` — `windowsScreenshot()`**
   Replace the nested loops that accumulate `physicalX` / `physicalY` by "fully left / fully above" rules with offsets derived from:

   - `minLogicalX`, `minLogicalY` from `desktopGeometry()` (union of `QScreen::geometry()`), and
   - `physicalX = round((screenGeom.x() - minLogicalX) * screenDpr)` (same for `y`).

2. **Same file — `cropToMonitor()` (Windows branch)**
   Align the crop rectangle with the same formula:

   - `cropX = round((targetGeometry.x() - minX) * targetDpr)`
   - `cropY = round((targetGeometry.y() - minY) * targetDpr)`
   with `minX` / `minY` consistent with the union used for compositing.

## Mixed-DPI caveat

Per-monitor scaling can still be non-linear across the virtual desktop. This approach is a practical step for common same-or-mixed-DPI setups; a follow-up could align with Win32 virtual-screen metrics or `QScreen::virtualGeometry()` if edge cases remain.

## How to verify

Test at least:

| Case | Check |
|------|--------|
| Secondary monitor with **negative X** | No black band; selection matches pixels. |
| **Different heights** (e.g. 1080p + 1200p) | Composite height matches union; no vertical misalignment. |
| Secondary **Y ≠ 0** (staggered row) | Left/right images align with physical layout; no row "collapse" to `y = 0`. |
| Swap **primary** display | Behaviour unchanged except expected OS focus differences. |

Also run **`flameshot gui`** (or full GUI capture) and confirm the frozen background matches both monitors.

## Files touched

- `src/utils/screengrabber.cpp` — `windowsScreenshot()`, `cropToMonitor()` (Windows `#else` branches).

No change required on Linux/macOS code paths for this specific bug.




