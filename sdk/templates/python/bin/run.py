#!/usr/bin/env python3
import json
import sys

# Flameshot writes the selected screenshot to standard input as PNG bytes.
image = sys.stdin.buffer.read()

# Replace this response with the result produced by your plugin. Flameshot
# currently supports "notification", "clipboard-text", and "none" results.
response = {
    "protocol_version": 1,
    "status": "success",
    "result": {
        "type": "notification",
        "title": "@PLUGIN_NAME@",
        "text": f"Received {len(image)} PNG bytes",
    },
}

# Standard output is reserved for exactly one JSON response. Write diagnostic
# messages to standard error with: print("message", file=sys.stderr)
print(json.dumps(response))
