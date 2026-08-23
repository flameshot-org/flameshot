#!/usr/bin/env python3
import json
import sys

image = sys.stdin.buffer.read()
print(json.dumps({
    "protocol_version": 1,
    "status": "success",
    "result": {
        "type": "notification",
        "title": "@PLUGIN_NAME@",
        "text": f"Received {len(image)} PNG bytes"
    }
}))
