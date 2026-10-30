#!/usr/bin/env python3
"""Check portal crop pixels without a display or a real screenshot portal.

Usage: python3 tests/wayland_capture_scale.py /path/to/flameshot
Requires dbus-run-session, PyGObject (Gio), Pillow, and Qt's offscreen plugin.
Runs on a private session bus and uses a temporary Flameshot configuration.
"""

import os
from pathlib import Path
import subprocess
import sys
import tempfile
import time


def main():
    if len(sys.argv) == 2:
        binary = str(Path(sys.argv[1]).resolve())
        return subprocess.call([
            "dbus-run-session", "--", sys.executable, __file__, binary,
            "--private-session",
        ])
    if len(sys.argv) != 3 or sys.argv[2] != "--private-session":
        sys.exit(__doc__)

    from gi.repository import Gio, GLib
    from PIL import Image, ImageChops

    binary = sys.argv[1]
    connection = Gio.bus_get_sync(Gio.BusType.SESSION, None)
    interface = Gio.DBusNodeInfo.new_for_xml("""
      <node><interface name="org.freedesktop.portal.Screenshot">
        <method name="Screenshot">
          <arg type="s" direction="in"/>
          <arg type="a{sv}" direction="in"/>
          <arg type="o" direction="out"/>
        </method>
      </interface></node>
    """).interfaces[0]
    request_interface = Gio.DBusNodeInfo.new_for_xml("""
      <node><interface name="org.freedesktop.portal.Request">
        <method name="Close"/>
      </interface></node>
    """).interfaces[0]
    context = GLib.MainContext.default()
    portal_image = None
    requests = []

    def close_request(conn, sender, path, iface, method, params, invocation):
        invocation.return_value(GLib.Variant("()", ()))

    def screenshot(conn, sender, path, iface, method, params, invocation):
        _, options = params.unpack()
        token = options["handle_token"]
        caller = sender.lstrip(":").replace(".", "_")
        request = f"/org/freedesktop/portal/desktop/request/{caller}/{token}"
        requests.append(connection.register_object(
            request, request_interface, close_request, None, None))
        invocation.return_value(GLib.Variant("(o)", (request,)))

        def respond():
            connection.emit_signal(
                sender, request, "org.freedesktop.portal.Request", "Response",
                GLib.Variant("(ua{sv})", (0, {
                    "uri": GLib.Variant("s", portal_image.as_uri()),
                })),
            )
            return GLib.SOURCE_REMOVE

        GLib.idle_add(respond)

    registration = connection.register_object(
        "/org/freedesktop/portal/desktop", interface, screenshot, None, None)
    connection.call_sync(
        "org.freedesktop.DBus", "/org/freedesktop/DBus",
        "org.freedesktop.DBus", "RequestName",
        GLib.Variant("(su)", ("org.freedesktop.portal.Desktop", 0)),
        GLib.VariantType.new("(u)"), Gio.DBusCallFlags.NONE, -1, None,
    )

    try:
        with tempfile.TemporaryDirectory(prefix="flameshot-portal-scale-") as temp:
            root = Path(temp)
            config = root / "config" / "flameshot"
            config.mkdir(parents=True)
            (config / "flameshot.ini").write_text(
                "[General]\nautoCloseIdleDaemon=true\nstartupLaunch=false\n"
                "showDesktopNotification=false\n")
            env = dict(os.environ, QT_QPA_PLATFORM="offscreen", QT_SCALE_FACTOR="2",
                       XDG_CONFIG_HOME=str(root / "config"),
                       XDG_CACHE_HOME=str(root / "cache"))
            for setting in ("QT_SCREEN_SCALE_FACTORS", "QT_FONT_DPI",
                            "QT_AUTO_SCREEN_SCALE_FACTOR", "QT_ENABLE_HIGHDPI_SCALING"):
                env.pop(setting, None)
            # Qt's default offscreen screen is 800x800 physical pixels.
            # At QT_SCALE_FACTOR=2 its logical size is 400x400, DPR=2.
            for session, scale in [("wayland", 1.25), ("wayland", 1.5),
                                   ("wayland", 2), ("x11", 1.25)]:
                env["XDG_SESSION_TYPE"] = session
                env.pop("WAYLAND_DISPLAY", None)
                env.pop("FLAMESHOT_NATIVE_OUTPUT", None)
                size = round(400 * scale)
                source = Image.new("RGB", (size, size))
                source.putdata([
                    ((x * 37 + y * 11) % 256, (x + y) % 2 * 255,
                     (x * 7 + y * 53) % 256)
                    for y in range(size) for x in range(size)
                ])
                portal_image = root / "portal.png"
                source.save(portal_image)
                destination = root / f"{session}-{scale}.png"
                with (root / "capture.log").open("w+") as log:
                    process = subprocess.Popen(
                        [binary, "screen", "-n", "0", "-p", str(destination)],
                        env=env, stdout=log, stderr=log,
                    )
                    try:
                        deadline = time.monotonic() + 20
                        while process.poll() is None and time.monotonic() < deadline:
                            while context.pending():
                                context.iteration(False)
                            time.sleep(0.01)
                        if process.poll() is None:
                            raise AssertionError("Screenshot request timed out")
                        log.seek(0)
                        assert process.returncode == 0, log.read()
                    finally:
                        if process.poll() is None:
                            process.terminate()
                            process.wait(timeout=5)
                with Image.open(destination) as result:
                    expected_size = size if session == "wayland" else 800
                    assert result.size == (expected_size, expected_size), (
                        session, scale, result.size, expected_size)
                    if session == "wayland":
                        difference = ImageChops.difference(result.convert("RGB"), source)
                        assert difference.getbbox() is None, (session, scale, "pixels changed")
                print(f"PASS: {session}, portal scale {scale}, {expected_size}x{expected_size}")
    finally:
        connection.unregister_object(registration)
        for request in requests:
            connection.unregister_object(request)
    return 0


if __name__ == "__main__":
    sys.exit(main())
