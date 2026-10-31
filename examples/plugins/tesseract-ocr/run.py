#!/usr/bin/env python3
"""Flameshot action plugin using the local Tesseract executable."""

import json
import subprocess
import sys


def result(value):
    print(json.dumps(value, ensure_ascii=False))


image = sys.stdin.buffer.read()
try:
    completed = subprocess.run(
        ["tesseract", "stdin", "stdout", *sys.argv[1:]],
        input=image,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        timeout=28,
        check=False,
    )
except (OSError, subprocess.TimeoutExpired) as error:
    result({"protocol_version": 1, "status": "error", "message": str(error)})
    raise SystemExit(0)

if completed.returncode != 0:
    message = completed.stderr.decode("utf-8", errors="replace").strip()
    result({
        "protocol_version": 1,
        "status": "error",
        "message": message or f"Tesseract exited with {completed.returncode}",
    })
else:
    text = completed.stdout.decode("utf-8", errors="replace").strip()
    if text:
        result({
            "protocol_version": 1,
            "status": "success",
            "result": {"type": "clipboard-text", "text": text},
        })
    else:
        result({
            "protocol_version": 1,
            "status": "error",
            "message": "Tesseract did not recognize any text.",
        })
