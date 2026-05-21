# Tests

This directory currently holds two **manual** smoke-test scripts. There is no
automated unit/integration test suite yet — see backlog stories S2.1–S2.6 in
`docs/backlog/` for the plan.

## Manual scripts

| Script | What it covers | How to run |
|---|---|---|
| `action_options.sh` | Final-action options for `flameshot` commands (`--path`, `--clipboard`, `--raw`, `--pin`, `--print-geometry`) across capture modes (`full`, `screen`, `gui`). Requires watching the screen and confirming notifications. | `./tests/action_options.sh /path/to/built/flameshot` |
| `path_option.sh` | `-p / --path` handling: relative paths, absolute paths, redundancy removal, missing directories, suffix inference. | Start a matching `flameshot` daemon first, then `./tests/path_option.sh /path/to/built/flameshot` |

Both scripts shell out to a real `flameshot` binary. Build first with the
instructions in the root `README.md`, then point each script at
`build/src/flameshot` (or your platform's equivalent).

### Dependencies

- `action_options.sh` uses ImageMagick's `display` for `--raw` verification.
  Install via your distro (`apt install imagemagick`, `brew install
  imagemagick`, etc.) before running.
- Both scripts expect a working desktop session — they will not run usefully
  inside a CI container or over SSH without an X/Wayland display.

## Why these aren't in CI

They require human verification (notifications, on-screen pins, image
display windows). Replacing them with assertion-driven tests is tracked
under backlog story S2.7. Until then they serve as recipes for maintainers
to run by hand before tagging a release.

## Adding new tests

Once `tests/CMakeLists.txt` lands (story S2.1), new tests will live in
subdirectories per subsystem (`tests/config/`, `tests/utils/`,
`tests/integration/`) and be discovered automatically by `ctest`. Refer to
`docs/CONTRIBUTING.md` for the test-writing workflow.
