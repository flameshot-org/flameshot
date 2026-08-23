# Plugin platform architecture

Flameshot treats built-in tools and external plugins as tools with stable
string identifiers. They share discovery, ordering, visibility, shortcuts, and
toolbar presentation, while their execution remains deliberately separate.

```text
Capture UI
    |
    v
ToolRegistry
    |-- BuiltIn provider ------> ToolFactory ------> native CaptureTool
    `-- ExternalAction provider -> PluginActionTool -> ActionPluginRunner
                                                   -> Flameshot core services
```

## Core platform

The core owns functionality that depends on the desktop session or Flameshot's
editor state:

- screen capture and selection;
- the capture model and undo/redo;
- clipboard access and desktop notifications;
- tool ordering, visibility, and shortcuts;
- plugin discovery, validation, process lifetime, and timeouts.
- package installation, writable plugin configuration, and result validation.

Plugins return data to the host instead of implementing desktop integration
themselves. For example, an OCR plugin writes UTF-8 text to standard output and
Flameshot copies it through its clipboard service. This keeps Wayland-specific
behavior in one place.

## Tool registry

Every tool has a stable ID. Built-in tools use IDs such as
`org.flameshot.pencil`; external tools use the ID from their manifest, such as
`org.flameshot.tesseract-ocr`.

The existing `CaptureTool::Type` values remain as a compatibility bridge for
old integer-based settings and native editor logic. External tools are not
assigned new enum values. This allows any number of plugins without changing
or recompiling the enum.

The toolbar consumes registry descriptors and does not need a separate plugin
chooser. The configuration UI uses the same registry, so external actions can
be enabled or disabled like built-in buttons. User overrides are stored below
`Plugins/<plugin-id>` and are kept separate from the installed manifest.

## External action provider

Version 1 runs plugins as subprocesses. A selected capture is encoded as PNG
and sent to standard input. The host waits asynchronously, enforces the
manifest timeout, validates the versioned JSON result, and handles crashes and
unsuccessful exit codes without loading plugin code into the Flameshot process.

The capture window is hidden while an action runs and closes only after the
child process finishes. This keeps the Qt event loop alive without showing an
intermediate plugin window.

## Distribution and SDK

The core plugin manager owns XDG discovery, package validation, staged installs,
enablement, removal, and diagnostics. A `.flameshot-plugin` is intentionally an
opaque extension over a conventional compressed TAR archive, keeping packages
easy to inspect and the runtime dependency surface small.

The language-neutral SDK mirrors the host validator and includes manifest and
result schemas. Plugin authors can start from Python, shell, Rust, or C++
templates, test the standard-input/result contract, and create deterministic
packages. The subprocess boundary means other languages require no Flameshot
library or C ABI.

## Future providers

The registry is intentionally provider-based. Later versions can add image
transform plugins (PNG input and PNG output), exporters (image to URL or file),
and eventually interactive editor tools. Interactive plugins need a separate
protocol for pointer events, preview rendering, and undo/redo and should not be
added to the action protocol.
