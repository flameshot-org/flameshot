# Action plugins (experimental)

Action plugins add a button directly to the Flameshot capture toolbar. When the
button is selected, Flameshot starts the plugin asynchronously, sends the
selected image as PNG on standard input, and applies the plugin's structured
result through core services such as the clipboard and desktop notifications.

This API does not load third-party code into the Flameshot process and does not
yet provide interactive annotation tools.

## Install and manage plugins

The configuration window has **Install Plugin…** and **Remove Plugin** buttons.
The same operations are available from the command line:

```sh
flameshot plugins install org.example.tool.flameshot-plugin
flameshot plugins list
flameshot plugins disable org.example.tool
flameshot plugins enable org.example.tool
flameshot plugins doctor
flameshot plugins remove org.example.tool
```

User plugins follow the XDG Base Directory specification and are installed in
`${XDG_DATA_HOME:-~/.local/share}/flameshot/plugins/<plugin-id>`. Flameshot also
searches each XDG system data directory. The prototype location
`~/.config/flameshot/plugins` remains a compatibility search root.

A `.flameshot-plugin` package is a gzip-compressed TAR archive whose
`metadata.json` is at the archive root. The installer rejects absolute and
parent paths, links, special files, packages over 32 MiB, extracted content over
64 MiB, and archives containing over 1000 files. Plugins are installed through
a staging directory and activated with an atomic rename.

## Create a plugin

The installed SDK provides templates for Python, POSIX shell, Rust, and C++:

```sh
flameshot-plugin create org.example.my-tool --language python
flameshot-plugin test org.example.my-tool --image screenshot.png
flameshot-plugin validate org.example.my-tool
flameshot-plugin pack org.example.my-tool
flameshot plugins install org.example.my-tool.flameshot-plugin
```

JSON schemas for the manifest and result protocol are installed in
`share/flameshot/sdk/schemas`.

## Manifest API 1

```json
{
  "api_version": 1,
  "type": "action",
  "id": "org.example.ocr",
  "name": "Local OCR",
  "description": "Recognize text and copy it to the clipboard",
  "icon": "ocr.svg",
  "toolbar": {
    "visible": true,
    "order": 175,
    "shortcut": "Ctrl+Shift+O"
  },
  "command": {
    "executable": "./bin/run",
    "arguments": [],
    "input": "png-stdin",
    "output": "json",
    "timeout_ms": 30000
  }
}
```

Required fields are `api_version`, `type`, `id`, `name`, and
`command.executable`. API 1 accepts only `type: action` and `png-stdin` input.
The toolbar fields control direct visibility, relative order, and shortcut.

The recommended `json` output is a single UTF-8 object:

```json
{
  "protocol_version": 1,
  "status": "success",
  "result": {
    "type": "clipboard-text",
    "text": "recognized text"
  }
}
```

Success result types are `clipboard-text`, `notification` (with `text` and an
optional `title`), and `none`. A controlled failure is returned as
`{"protocol_version":1,"status":"error","message":"..."}`. The legacy
output modes `clipboard-text` and `none` remain supported for API 1.

Flameshot supplies these environment variables:

- `FLAMESHOT_PLUGIN_API_VERSION`: currently `1`;
- `FLAMESHOT_PLUGIN_ID`: the stable manifest ID;
- `FLAMESHOT_PLUGIN_DIR`: the read-only installation directory;
- `FLAMESHOT_PLUGIN_CONFIG_DIR`: a writable per-plugin XDG config directory.

Standard error is used for diagnostics when a process fails. Flameshot enforces
the configured timeout, 1 MiB standard-output limit, and 256 KiB standard-error
limit.

## Security and compatibility

Plugins run with the same user privileges as Flameshot. Package validation and
process isolation reduce accidental damage, but they are not a security
sandbox. Install only plugins you trust. A subprocess protocol avoids a fragile
C++/Qt ABI and keeps plugin crashes outside the main application.
