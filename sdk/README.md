# Flameshot action-plugin SDK

This guide starts with a working plugin and explains the protocol only after
you have run it. No knowledge of the Flameshot C++ code is required.

The SDK needs Python 3.10 or newer. A plugin itself may be written in any
language that can read bytes from standard input and print JSON.

## The idea in one minute

An action plugin is a small external program connected to a toolbar button:

```text
selected screenshot                         plugin result
      PNG                                        JSON
       |                                           |
       v                                           v
   Flameshot  -------- standard input -------->  plugin
   Flameshot  <------- standard output --------  plugin
       |
       `---- copies text or displays a notification
```

Flameshot owns the screenshot UI, clipboard, notifications, plugin timeout,
and error handling. The plugin receives one PNG image, performs one action,
and returns one result.

For example, an OCR plugin does not access the clipboard. It returns recognized
text and asks Flameshot to copy it.

## Five-minute quick start

### 1. Create a Python plugin

Plugin IDs use reverse-domain notation so they do not collide with other
plugins. Replace `example.org` with a domain or namespace you control when
publishing a real plugin.

```sh
flameshot-plugin create org.example.hello-flameshot --language python
cd org.example.hello-flameshot
```

The generated project contains only two required files:

```text
org.example.hello-flameshot/
├── metadata.json   # tells Flameshot how the plugin should appear and run
└── bin/
    └── run.py      # receives the PNG and returns a JSON result
```

### 2. Test it without installing it

```sh
flameshot-plugin test .
```

The SDK sends a tiny built-in PNG to the plugin. The result should look like:

```json
{
  "protocol_version": 1,
  "status": "success",
  "result": {
    "type": "notification",
    "title": "Hello Flameshot",
    "text": "Received 68 PNG bytes"
  }
}
```

To test with a real screenshot:

```sh
flameshot-plugin test . --image /path/to/screenshot.png
```

### 3. Change what the plugin does

Open `bin/run.py`. The important part is:

```python
import json
import sys

# Flameshot sends the selected screenshot here as exact PNG bytes.
image = sys.stdin.buffer.read()

# Replace this object with the result your plugin should return.
response = {
    "protocol_version": 1,
    "status": "success",
    "result": {
        "type": "notification",
        "title": "My plugin",
        "text": f"The image contains {len(image)} bytes",
    },
}

# Standard output is reserved for one JSON response.
print(json.dumps(response))
```

Your image processing belongs between reading `image` and creating `response`.
Write diagnostic messages to standard error, not standard output:

```python
print("Loading model...", file=sys.stderr)
```

### 4. Validate and package it

```sh
flameshot-plugin validate .
flameshot-plugin pack .
```

This creates:

```text
../org.example.hello-flameshot.flameshot-plugin
```

By default, `pack` places the package next to the plugin project, never inside
it. The package is a compressed archive containing the plugin files. It can be
inspected with normal TAR tools, but users should install it through Flameshot
so its structure and manifest are checked.

### 5. Install and use it

```sh
cd ..
flameshot plugins install org.example.hello-flameshot.flameshot-plugin
flameshot plugins list
flameshot plugins doctor
```

Start a normal capture. The plugin appears as its own toolbar button. Selecting
the button runs the plugin with the currently selected screenshot area.

The same package can be installed from **Configuration > Button Selection >
Install Plugin...**.

To disable or remove it:

```sh
flameshot plugins disable org.example.hello-flameshot
flameshot plugins enable org.example.hello-flameshot
flameshot plugins remove org.example.hello-flameshot
```

## Understanding `metadata.json`

This file describes the plugin without executing it:

```json
{
  "api_version": 1,
  "type": "action",
  "id": "org.example.hello-flameshot",
  "name": "Hello Flameshot",
  "description": "Show information about the selected screenshot",
  "icon": "icon.svg",
  "toolbar": {
    "visible": true,
    "order": 1000,
    "shortcut": "Ctrl+Shift+H"
  },
  "command": {
    "executable": "./bin/run.py",
    "arguments": [],
    "input": "png-stdin",
    "output": "json",
    "timeout_ms": 30000
  }
}
```

| Field | Meaning |
| --- | --- |
| `api_version` | Manifest version understood by Flameshot. API v1 uses `1`. |
| `type` | Plugin category. API v1 supports only `action`. |
| `id` | Permanent, globally unique plugin ID. Do not change it after release. |
| `name` | Human-readable name shown in Flameshot. |
| `description` | Tooltip and configuration description. |
| `icon` | Optional relative path to an icon inside the plugin. |
| `toolbar.visible` | Whether a new installation enables the button by default. |
| `toolbar.order` | Relative position in the toolbar; lower numbers appear earlier. |
| `toolbar.shortcut` | Optional default shortcut. Users may override it. |
| `command.executable` | Program or plugin-relative executable to start. |
| `command.arguments` | String arguments passed to the executable. |
| `command.input` | API v1 supports `png-stdin`. |
| `command.output` | Use `json` for structured results. |
| `command.timeout_ms` | Maximum runtime from 1,000 to 300,000 milliseconds. |

Required fields are `api_version`, `type`, `id`, `name`, and
`command.executable`. The SDK validator and Flameshot loader apply the same
core constraints.

## Choosing a result

Every structured response contains `protocol_version` and `status`.

### Copy text to the clipboard

```json
{
  "protocol_version": 1,
  "status": "success",
  "result": {
    "type": "clipboard-text",
    "text": "Text produced by the plugin"
  }
}
```

### Display a desktop notification

```json
{
  "protocol_version": 1,
  "status": "success",
  "result": {
    "type": "notification",
    "title": "Plugin finished",
    "text": "The image was processed successfully"
  }
}
```

The notification `title` is optional. Flameshot uses the plugin name when it
is omitted.

### Finish without a visible result

```json
{
  "protocol_version": 1,
  "status": "success",
  "result": { "type": "none" }
}
```

### Report an expected error

```json
{
  "protocol_version": 1,
  "status": "error",
  "message": "No text was recognized"
}
```

Print an error response and exit with status `0` when the plugin ran normally
but could not produce a result. Flameshot can then display the useful message.
Use a non-zero exit status for a process failure; Flameshot displays standard
error in that case.

## Environment supplied by Flameshot

Flameshot starts the process in the plugin installation directory and provides:

| Variable | Purpose |
| --- | --- |
| `FLAMESHOT_PLUGIN_API_VERSION` | API version selected by the host. |
| `FLAMESHOT_PLUGIN_ID` | Stable plugin ID from the manifest. |
| `FLAMESHOT_PLUGIN_DIR` | Installation directory for read-only plugin assets. |
| `FLAMESHOT_PLUGIN_CONFIG_DIR` | Writable directory for this plugin's settings. |

Store user configuration only below `FLAMESHOT_PLUGIN_CONFIG_DIR`. Do not write
into `FLAMESHOT_PLUGIN_DIR`, because system packages may install it read-only.

## SDK command reference

```text
flameshot-plugin create ID [DIRECTORY] [--language LANGUAGE] [--name NAME]
flameshot-plugin validate DIRECTORY_OR_PACKAGE
flameshot-plugin test DIRECTORY [--image PNG]
flameshot-plugin pack DIRECTORY [-o PACKAGE] [--force]
```

### `create`

Copies a starter project and replaces its ID and display-name placeholders.
Available languages are `python`, `shell`, `rust`, and `cpp`.

### `validate`

Checks the file tree, executable permissions, manifest values, paths, size
limits, and package structure without installing or running the plugin.

### `test`

Runs an unpacked plugin, sends PNG data, applies the manifest timeout, and
validates structured JSON output. It prints the result instead of changing the
real clipboard or showing a desktop notification.

### `pack`

Creates a deterministic `.flameshot-plugin` archive next to the plugin project.
Use `-o` to select another location outside the project. Existing output is not
overwritten unless `--force` is specified.

## Other language templates

The protocol is language-neutral:

- `--language shell` creates a directly runnable POSIX shell example.
- `--language rust` creates a Cargo project and a `bin/run` launcher. Build it
  with `cargo build --release` before using `test` or packaging a working binary.
- `--language cpp` creates a small CMake project and launcher. Build it with
  `cmake -S . -B build && cmake --build build` before using `test`.

Native plugins do not link to Flameshot. They only implement the same stdin and
JSON contract, which avoids a C++ or Qt ABI dependency.

## OCR example

The repository includes `examples/plugins/tesseract-ocr`. It demonstrates how
a plugin can call another local program and return its output to Flameshot.

On Debian or Ubuntu, install the OCR engine and language data first:

```sh
sudo apt install tesseract-ocr tesseract-ocr-eng tesseract-ocr-deu
```

From a source checkout:

```sh
flameshot-plugin validate examples/plugins/tesseract-ocr
flameshot-plugin test examples/plugins/tesseract-ocr --image screenshot.png
flameshot-plugin pack examples/plugins/tesseract-ocr
flameshot plugins install org.flameshot.tesseract-ocr.flameshot-plugin
```

The example performs these steps:

1. Read the selected image from standard input.
2. Start the local `tesseract` executable.
3. Pass the image to Tesseract.
4. Return recognized text as a `clipboard-text` result.
5. Let Flameshot perform the actual clipboard operation.

## Troubleshooting

### `executable was not found` or `is not executable`

Use a plugin-relative path such as `./bin/run.py` and set its executable bit:

```sh
chmod +x bin/run.py
flameshot-plugin validate .
```

### `invalid result JSON`

Standard output must contain exactly one JSON object. Send logs to standard
error. A debug `print()` on standard output before the result will invalidate
the response.

### The plugin works in a terminal but not in Flameshot

Run `flameshot plugins doctor`, then test with the exact executable and files
that were packaged. Do not depend on the shell's current directory or aliases.
Use `FLAMESHOT_PLUGIN_DIR` for bundled assets.

### The plugin times out

Increase `command.timeout_ms` only when the action genuinely needs more time.
The allowed range is 1-300 seconds. Prefer loading models efficiently and
returning a controlled error when an external dependency stalls.

### The button is missing

```sh
flameshot plugins list
flameshot plugins enable org.example.hello-flameshot
flameshot plugins doctor
```

Restart Flameshot after installing a plugin if an already-open configuration or
capture window has cached the previous toolbar state.

### A package is rejected as unsafe

Packages may contain only regular files and directories. Symlinks, absolute
paths, parent traversal, special files, oversized content, and excessive file
counts are rejected intentionally.

## File locations

User plugins follow the XDG Base Directory specification:

```text
${XDG_DATA_HOME:-~/.local/share}/flameshot/plugins/<plugin-id>
```

Per-plugin writable configuration is placed below:

```text
${XDG_CONFIG_HOME:-~/.config}/flameshot/plugins/<plugin-id>
```

The JSON schemas installed with the SDK are in
`share/flameshot/sdk/schemas`. Source checkouts contain them in `sdk/schemas`.

## Security model

Plugins run as external processes, so a crash does not directly crash
Flameshot. They still run with the same user privileges as Flameshot. Package
validation and process isolation are not a security sandbox. Install only
plugins and packages you trust.

For the complete API and architecture reference, see
[`docs/action-plugins.md`](../docs/action-plugins.md) and
[`docs/plugin-architecture.md`](../docs/plugin-architecture.md).
