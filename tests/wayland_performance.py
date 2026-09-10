#!/usr/bin/env python3
"""Hyprland 0.55+ desktop integration check; temporarily replaces the clipboard.

Example: python3 tests/wayland_performance.py build/src/flameshot --fast
Use --screen 1 for a second monitor, --cancel to check Escape, or
--fail-tool grim / --fail-tool wl-copy to exercise the fallback backends.
"""

import argparse
import json
import os
from pathlib import Path
import struct
import subprocess
import tempfile
import time


def run(*args):
    return subprocess.run(args, check=True, capture_output=True, timeout=20).stdout


def image_dimensions(data):
    if data.startswith(b"\x89PNG\r\n\x1a\n"):
        return struct.unpack(">II", data[16:24])
    if data.startswith(b"\xff\xd8"):
        position = 2
        while position < len(data):
            if data[position] != 0xFF:
                break
            while position < len(data) and data[position] == 0xFF:
                position += 1
            marker = data[position]
            position += 1
            if marker in (0xDA, 0xD9):
                break
            if marker in range(0xD0, 0xD8) or marker == 0x01:
                continue
            length = int.from_bytes(data[position:position + 2], "big")
            if length < 2 or position + length > len(data):
                break
            if marker in (0xC0, 0xC1, 0xC2, 0xC3, 0xC5, 0xC6, 0xC7,
                          0xC9, 0xCA, 0xCB, 0xCD, 0xCE, 0xCF):
                height, width = struct.unpack(">HH", data[position + 3:position + 7])
                return width, height
            position += length
    raise RuntimeError("Clipboard is not a supported PNG/JPEG image")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("binary", type=Path)
    parser.add_argument("--hyprctl", default="/usr/bin/hyprctl")
    parser.add_argument("--screen", type=int, default=0)
    parser.add_argument("--fast", action="store_true")
    parser.add_argument("--cancel", action="store_true")
    parser.add_argument("--fail-tool", choices=("grim", "wl-copy"))
    options = parser.parse_args()
    if options.fail_tool and not options.fast:
        parser.error("--fail-tool requires --fast")

    def windows():
        return [w for w in json.loads(run(options.hyprctl, "clients", "-j"))
                if w["class"].lower() in ("flameshot", "org.flameshot.flameshot")]

    def send_key(address, mods, key):
        for state in ("down", "up"):
            expression = (
                "hl.dsp.send_key_state({mods=" + json.dumps(mods)
                + ",key=" + json.dumps(key) + ",state=" + json.dumps(state)
                + ",window=" + json.dumps("address:" + address) + "})"
            )
            result = run(options.hyprctl, "dispatch", expression)
            if result.strip() != b"ok":
                # Copy/Escape may destroy the editor on key-down before the
                # compositor receives the matching release.
                if (state == "up" and b"send_key_state: window not found" in result
                        and not any(w["address"] == address for w in windows())):
                    continue
                raise RuntimeError(result.decode())

    if windows():
        raise RuntimeError("Close existing Flameshot windows before this test")

    # wl-copy forks a persistent owner; do not hold its inherited stdout/stderr
    # pipes open through subprocess.communicate().
    marker = b"Flameshot Wayland integration test"
    subprocess.run(["wl-copy", "--type", "text/plain"], input=marker,
                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
                   check=True, timeout=3)
    environment = os.environ.copy()
    environment["QT_QPA_PLATFORM"] = "wayland"
    for name in ("QT_SCALE_FACTOR", "QT_SCREEN_SCALE_FACTORS",
                 "QT_AUTO_SCREEN_SCALE_FACTOR", "FLAMESHOT_USE_GRIM",
                 "FLAMESHOT_USE_WL_COPY"):
        environment.pop(name, None)
    if options.fast:
        environment.update(FLAMESHOT_USE_GRIM="1", FLAMESHOT_USE_WL_COPY="1")

    with tempfile.TemporaryDirectory(prefix="flameshot-test-") as directory:
        if options.fail_tool:
            stub = Path(directory) / options.fail_tool
            stub.write_text("#!/bin/sh\nexit 1\n")
            stub.chmod(0o700)
            environment["PATH"] = directory + os.pathsep + environment["PATH"]
        with tempfile.TemporaryFile() as log:
            start = time.monotonic()
            process = subprocess.Popen(
                [str(options.binary.resolve()), "screen", "--number",
                 str(options.screen), "--edit", "--region", "800x500+100+100"],
                env=environment, stdout=log, stderr=log,
            )
            address = None
            completed = False
            try:
                while time.monotonic() - start < 20:
                    owned = [w for w in windows() if w["pid"] == process.pid]
                    if owned:
                        address = owned[0]["address"]
                        break
                    if process.poll() is not None:
                        raise RuntimeError("Flameshot exited before showing the editor")
                    time.sleep(0.01)
                if address is None:
                    raise RuntimeError("No editor appeared within 20 seconds")
                overlay_ms = round((time.monotonic() - start) * 1000)
                run(options.hyprctl, "dispatch",
                    "hl.dsp.focus({window=" + json.dumps("address:" + address) + "})")
                # Allow the compositor to deliver focus before the test key.
                time.sleep(0.1)
                copied = time.monotonic()
                send_key(address, "" if options.cancel else "CTRL",
                         "Escape" if options.cancel else "c")
                code = process.wait(timeout=10)
                if options.cancel:
                    if run("wl-paste", "--no-newline", "--type", "text/plain") != marker:
                        raise RuntimeError("Escape overwrote the clipboard")
                    result = {"overlay_ms": overlay_ms, "cancel_preserved_clipboard": True}
                else:
                    if code != 0:
                        raise RuntimeError("Capture failed with exit code " + str(code))
                    mime = None
                    while time.monotonic() - copied < 10:
                        types = run("wl-paste", "--list-types").splitlines()
                        mime = next((m for m in ("image/png", "image/jpeg")
                                     if m.encode() in types), None)
                        if mime:
                            break
                        time.sleep(0.01)
                    if mime is None:
                        raise RuntimeError("No clipboard image arrived within 10 seconds")
                    image = run("wl-paste", "--type", mime)
                    copy_ms = round((time.monotonic() - copied) * 1000)
                    if image_dimensions(image) != (800, 500):
                        raise RuntimeError("Selection dimensions changed")
                    if run("wl-paste", "--type", mime) != image:
                        raise RuntimeError("Repeated paste returned different bytes")
                    result = {"overlay_ms": overlay_ms, "clipboard_ms": copy_ms,
                              "mime": mime, "image_bytes": len(image), "dimensions": [800, 500],
                              "repeated_paste": True}
                if any(w["address"] == address for w in windows()):
                    raise RuntimeError("Editor remained open after copy/cancel")
                completed = True
                print(json.dumps(result))
            finally:
                if process.poll() is None:
                    if address and any(w["address"] == address for w in windows()):
                        send_key(address, "", "Escape")
                    try:
                        process.wait(timeout=3)
                    except subprocess.TimeoutExpired:
                        process.terminate()
                        process.wait(timeout=3)
                if not completed:
                    log.seek(0)
                    print(log.read().decode(errors="replace"))


if __name__ == "__main__":
    main()
