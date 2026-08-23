#!/usr/bin/env python3

import os
from pathlib import Path
import subprocess
import sys
import tarfile
import tempfile


def run(command, environment, expected=0):
    completed = subprocess.run(
        command,
        env=environment,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        check=False,
    )
    if completed.returncode != expected:
        print("command:", command, file=sys.stderr)
        print(completed.stdout, file=sys.stderr)
        print(completed.stderr, file=sys.stderr)
        raise SystemExit(
            f"expected exit {expected}, received {completed.returncode}"
        )
    return completed


def main():
    flameshot = Path(sys.argv[1]).resolve()
    sdk = Path(sys.argv[2]).resolve()
    with tempfile.TemporaryDirectory(prefix="flameshot-plugin-cli-") as temporary:
        root = Path(temporary)
        environment = os.environ.copy()
        environment.update(
            {
                "XDG_DATA_HOME": str(root / "data"),
                "XDG_CONFIG_HOME": str(root / "config"),
                "XDG_DATA_DIRS": "/usr/local/share:/usr/share",
                "QT_QPA_PLATFORM": "offscreen",
                "LC_ALL": "C",
            }
        )
        source = root / "org.example.cli-test"
        package = root / "cli-test.flameshot-plugin"
        run([str(sdk), "create", "org.example.cli-test", str(source)], environment)
        run([str(sdk), "test", str(source)], environment)
        run([str(sdk), "pack", str(source), "-o", str(package)], environment)
        run([str(sdk), "validate", str(package)], environment)

        installed = run(
            [str(flameshot), "plugins", "install", str(package)], environment
        )
        if "org.example.cli-test" not in installed.stdout:
            raise SystemExit("install output does not identify the plugin")
        listed = run([str(flameshot), "plugins", "list"], environment)
        if "enabled  org.example.cli-test" not in listed.stdout:
            raise SystemExit("installed plugin is not listed as enabled")
        run(
            [str(flameshot), "plugins", "disable", "org.example.cli-test"],
            environment,
        )
        listed = run([str(flameshot), "plugins", "list"], environment)
        if "disabled org.example.cli-test" not in listed.stdout:
            raise SystemExit("plugin was not disabled")
        run(
            [str(flameshot), "plugins", "enable", "org.example.cli-test"],
            environment,
        )
        run([str(flameshot), "plugins", "doctor"], environment)
        run(
            [str(flameshot), "plugins", "remove", "org.example.cli-test"],
            environment,
        )
        if (root / "data/flameshot/plugins/org.example.cli-test").exists():
            raise SystemExit("plugin directory remains after removal")

        unsafe = root / "unsafe.flameshot-plugin"
        with tarfile.open(unsafe, "w:gz") as archive:
            info = tarfile.TarInfo("../outside")
            info.size = 0
            archive.addfile(info)
        run(
            [str(flameshot), "plugins", "install", str(unsafe)],
            environment,
            expected=1,
        )
        if (root / "outside").exists():
            raise SystemExit("unsafe package escaped the staging directory")


if __name__ == "__main__":
    main()
