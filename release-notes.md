# Release Notes

## Windows multi-monitor capture

This change set fixes the Windows graphical capture flow when Flameshot is used with multiple monitors, especially when monitors have different resolutions, aspect ratios, or DPI scaling.

### Problem summary

There were two distinct failure modes:

1. `Capture all displays`
   The darkened overlay and selection surface could become distorted or offset because the desktop was being composed from Qt logical screen geometry instead of the physical monitor layout reported by Windows.

2. `Ask which monitor to capture`
   The monitor picker could show previews in the wrong order, bind the wrong monitor to a preview card, or always open the editing overlay on the left-most monitor regardless of the user's choice.

### What was implemented

#### 1. New Windows configuration option

A Windows-only setting was added to let the user choose how graphical capture behaves on multi-monitor setups:

- `Ask which monitor to capture`
- `Capture all displays`

Affected areas:

- `src/config/generalconf.cpp`
- `src/config/generalconf.h`
- `src/utils/confighandler.cpp`
- `src/utils/confighandler.h`
- `src/core/capturerequest.cpp`
- `src/core/capturerequest.h`

#### 2. Physical desktop composition for `Capture all displays`

The full-desktop graphical overlay now uses the physical monitor layout from Windows instead of relying only on Qt logical geometry.

This fixes cases where one monitor looked stretched, shifted, or partially darkened when monitors had different size or DPI settings.

Affected areas:

- `src/utils/screengrabber.cpp`
- `src/utils/screengrabber.h`
- `src/widgets/capture/capturewidget.cpp`

Implementation highlights:

- Enumerate native Windows monitor rectangles.
- Build the full desktop screenshot using those physical rectangles.
- Position the graphical overlay using the same physical desktop geometry.
- Reuse those same rectangles for selection and painting logic.

#### 3. Stable monitor mapping for the legacy picker

The legacy picker path was corrected so the same monitor identity is used consistently across:

- preview generation
- on-screen ordering
- selected monitor storage
- the final editor window position

Affected areas:

- `src/widgets/capturelauncher.cpp`
- `src/utils/monitorpreview.cpp`
- `src/utils/monitorpreview.h`
- `src/utils/screengrabber.cpp`
- `src/widgets/capture/capturewidget.cpp`

Implementation highlights:

- Use a stable ordered monitor list for preview cards.
- Bind each card to the actual `QScreen*` it represents.
- Keep the selected `QScreen*` through the whole flow instead of recalculating from a loosely related index later.
- Use the selected screen when opening the editing overlay so it opens on the chosen monitor rather than defaulting to the left-most desktop origin.

### Sequence diagram

```mermaid
sequenceDiagram
    actor User
    participant Config as GeneralConf
    participant Launcher as CaptureLauncher
    participant Grabber as ScreenGrabber
    participant Widget as CaptureWidget
    participant Win as Windows Monitor Layout

    User->>Config: Choose multi-monitor mode

    alt Ask which monitor to capture
        User->>Launcher: Start graphical capture
        Launcher->>Grabber: Request ordered monitor list
        Grabber->>Win: Read monitor geometry / screens
        Win-->>Grabber: Ordered monitor data
        Grabber-->>Launcher: Preview metadata + screen binding
        Launcher-->>User: Show monitor picker cards
        User->>Launcher: Click one monitor
        Launcher->>Grabber: Store selected QScreen
        Launcher->>Widget: Open graphical editor
        Widget->>Grabber: Query selected screen
        Grabber-->>Widget: Selected QScreen
        Widget->>Widget: Move editor to chosen monitor
    else Capture all displays
        User->>Launcher: Start graphical capture
        Launcher->>Widget: Open full desktop editor
        Widget->>Grabber: Grab full desktop
        Grabber->>Win: Enumerate physical monitor rectangles
        Win-->>Grabber: Native monitor layout
        Grabber-->>Widget: Full screenshot + physical desktop geometry
        Widget->>Widget: Build dark overlay using physical monitor layout
        Widget-->>User: Single selection surface across all displays
    end
```

### Result

On Windows:

- `Capture all displays` now respects the real monitor layout and no longer distorts the darkened overlay on mixed monitor setups.
- `Ask which monitor to capture` now preserves correct preview order and opens the editor on the monitor actually selected by the user.

### Notes about platform scope

This work is primarily Windows-specific. The sensitive runtime changes are isolated to the Windows capture path, while Linux and macOS continue using their existing platform-specific behavior.
