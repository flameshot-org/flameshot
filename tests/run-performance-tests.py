#!/usr/bin/env python3
"""Build regression tests against the exact objects in an existing Linux Ninja build.

Usage: python3 tests/run-performance-tests.py [build-directory] [--benchmark]
Requires a completed Flameshot build, Python 3, pkg-config and Qt6Test.
Uses an offscreen display and temporary settings; never captures the desktop.
"""
import os
from pathlib import Path
import shlex
import subprocess
import sys
import tempfile

root = Path(__file__).resolve().parents[1]
build = Path(sys.argv[1] if len(sys.argv) > 1 else root / "build").resolve()
output = build / "performance-tests"
output.mkdir(exist_ok=True)
commands = subprocess.check_output(
    ["/usr/bin/ninja", "-C", str(build), "-t", "commands", "flameshot"], text=True
).splitlines()
compile_line = next(line for line in commands if " -c " in line and
                    line.endswith("/widgets/capture/capturewidget.cpp"))
compile_args = shlex.split(compile_line)
filtered = []
skip = False
for arg in compile_args:
    if skip:
        skip = False
    elif arg in ("-o", "-c", "-MF", "-MT", "-MQ"):
        skip = True
    elif arg not in ("-MD", "-MMD"):
        filtered.append(arg)
qt_flags = shlex.split(subprocess.check_output(
    ["pkg-config", "--cflags", "--libs", "Qt6Test"], text=True))
test_object = output / "performance-regression.o"
subprocess.run(filtered + qt_flags + ["-UNDEBUG", "-c",
               str(root / "tests/performance-regression.cpp"), "-o", str(test_object)],
               cwd=build, check=True)
link_args = []
for line in commands:
    chunks = [[]]
    for arg in shlex.split(line):
        if arg == "&&":
            chunks.append([])
        else:
            chunks[-1].append(arg)
    for chunk in chunks:
        if "-o" in chunk and chunk[chunk.index("-o") + 1] == "src/flameshot":
            link_args = chunk
if not link_args:
    raise SystemExit("Cannot locate the Flameshot link command in the Ninja build")
executable = output / "performance-regression"
link_args[link_args.index("-o") + 1] = str(executable)
link_args = [str(test_object) if arg.endswith("/main.cpp.o") else arg
             for arg in link_args if not arg.startswith("-Wl,--dependency-file=")]
subprocess.run(link_args + qt_flags, cwd=build, check=True)
with tempfile.TemporaryDirectory(prefix="flameshot-tests-") as settings:
    env = os.environ.copy()
    env.update(QT_QPA_PLATFORM="offscreen", XDG_CONFIG_HOME=settings,
               XDG_CACHE_HOME=settings, XDG_RUNTIME_DIR=settings)
    for name in ("QT_PLUGIN_PATH", "QT_QPA_PLATFORM_PLUGIN_PATH", "QT_QPA_PLATFORMTHEME",
                 "GTK_PATH", "GTK_MODULES", "GSETTINGS_SCHEMA_DIR", "LD_PRELOAD", "LD_LIBRARY_PATH"):
        env.pop(name, None)
    result = subprocess.run([str(executable)] + sys.argv[2:], env=env,
                            text=True, capture_output=True)
    (output / "results.txt").write_text(result.stdout + result.stderr)
    print(result.stdout, end="")
    if result.returncode:
        print(result.stderr, file=sys.stderr, end="")
    sys.exit(result.returncode)
