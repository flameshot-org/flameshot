#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
    printf 'Usage: %s /path/to/flameshot\n' "$0" >&2
    exit 2
fi

for command in Xvfb i3 i3-msg ps xterm xdotool xprop xwininfo jq file; do
    command -v "$command" >/dev/null ||
        { printf 'Missing test dependency: %s\n' "$command" >&2; exit 2; }
done

test_dir="$(mktemp -d)"
xvfb_pid=
i3_pid=
xterm_pid=
capture_pid=
events_pid=
cleanup() {
    for pid in "$capture_pid" "$events_pid" "$xterm_pid" "$i3_pid" "$xvfb_pid"; do
        if [[ -n "$pid" ]]; then
            kill "$pid" 2>/dev/null || true
            wait "$pid" 2>/dev/null || true
        fi
    done
    rm -rf -- "$test_dir"
}
trap cleanup EXIT

source_binary="$(readlink -f -- "$1")"
if [[ ! -x "$source_binary" ]]; then
    printf 'Flameshot executable not found: %s\n' "$1" >&2
    exit 2
fi

mkdir -p "$test_dir/bin" "$test_dir/config/flameshot" "$test_dir/runtime"
chmod 700 "$test_dir/runtime"
install -m 755 "$source_binary" "$test_dir/bin/flameshot"
cat >"$test_dir/bin/flameshot.ini" <<'EOF'
[General]
useX11LegacyScreenshot=true
captureActiveMonitor=true
EOF
cp "$test_dir/bin/flameshot.ini" "$test_dir/config/flameshot/flameshot.ini"
printf 'font pango:monospace 8\n' >"$test_dir/i3.conf"

Xvfb -displayfd 3 -screen 0 1280x720x24 -nolisten tcp -ac \
    3>"$test_dir/display" >"$test_dir/xvfb.log" 2>&1 &
xvfb_pid=$!
for ((attempt = 0; attempt < 100; attempt++)); do
    [[ -s "$test_dir/display" ]] && break
    sleep 0.05
done
if [[ ! -s "$test_dir/display" ]]; then
    cat "$test_dir/xvfb.log" >&2
    printf 'Xvfb did not start\n' >&2
    exit 1
fi

export DISPLAY=":$(cat "$test_dir/display")"
export QT_QPA_PLATFORM=xcb
export XDG_CONFIG_HOME="$test_dir/config"
export XDG_DATA_HOME="$test_dir/data"
export XDG_CACHE_HOME="$test_dir/cache"
export XDG_RUNTIME_DIR="$test_dir/runtime"
export XDG_SESSION_TYPE=x11
export DESKTOP_SESSION=i3
export DBUS_SESSION_BUS_ADDRESS="unix:path=$test_dir/no-session-bus"
unset I3SOCK KDE_FULL_SESSION GNOME_DESKTOP_SESSION_ID

XDG_CURRENT_DESKTOP=i3 i3 -c "$test_dir/i3.conf" >"$test_dir/i3.log" 2>&1 &
i3_pid=$!
for ((attempt = 0; attempt < 100; attempt++)); do
    if i3-msg -t get_version >/dev/null 2>&1; then
        break
    fi
    sleep 0.05
done
if ! i3-msg -t get_version >/dev/null 2>&1; then
    cat "$test_dir/i3.log" >&2
    printf 'i3 did not start\n' >&2
    exit 1
fi

test_class=FullscreenTestWindow
xterm -class "$test_class" -title "$test_class" \
    -e sh -c 'printf "FLAMESHOT OCR TEST\n"; sleep 600' \
    >"$test_dir/xterm.log" 2>&1 &
xterm_pid=$!
for ((attempt = 0; attempt < 100; attempt++)); do
    test_window="$(xdotool search --onlyvisible --class "$test_class" 2>/dev/null | head -n 1 || true)"
    [[ -n "$test_window" ]] && break
    sleep 0.05
done
if [[ -z "${test_window:-}" ]]; then
    cat "$test_dir/xterm.log" >&2
    printf 'Test window did not appear\n' >&2
    exit 1
fi
i3-msg "[class=\"$test_class\"] focus" >/dev/null

fail() {
    printf 'i3 fullscreen overlay test: %s\n' "$1" >&2
    if [[ -n "$capture_pid" ]]; then
        cat "$test_dir/flameshot.log" >&2 2>/dev/null || true
    fi
    exit 1
}

get_mode() {
    i3-msg -t get_tree | jq -er --arg class "$test_class" '
      [.. | objects |
       select(.window != null and .window_properties.class == $class)][0]
      .fullscreen_mode'
}

assert_mode() {
    local expected="$1"
    local actual
    actual="$(get_mode)" || fail "could not read test-window fullscreen mode"
    [[ "$actual" == "$expected" ]] ||
        fail "expected fullscreen mode $expected, got $actual"
}

has_flameshot_container() {
    i3-msg -t get_tree | jq -e '
      [.. | objects |
       select(.window != null and
              ((.window_properties.class // "") | ascii_downcase |
               contains("flameshot")))] | length > 0' >/dev/null
}

find_flameshot_window() {
    local window
    while IFS= read -r window; do
        if xprop -id "$window" WM_CLASS 2>/dev/null | grep -qi flameshot; then
            printf '%s\n' "$window"
            return 0
        fi
    done < <(xdotool search --onlyvisible --name '.*' 2>/dev/null || true)
    return 1
}

wait_for_flameshot_window() {
    local window
    for ((attempt = 0; attempt < 100; attempt++)); do
        if window="$(find_flameshot_window)"; then
            printf '%s\n' "$window"
            return 0
        fi
        if ! kill -0 "$capture_pid" 2>/dev/null; then
            cat "$test_dir/flameshot.log" >&2
            fail "Flameshot exited before opening its capture overlay"
        fi
        sleep 0.05
    done
    fail "Flameshot capture overlay did not appear"
}

start_events() {
    local path="$1"
    i3-msg -t subscribe -m '["window"]' >"$path" 2>"$test_dir/i3-msg.log" &
    events_pid=$!
    for ((attempt = 0; attempt < 100; attempt++)); do
        ps -p "$events_pid" >/dev/null && return 0
        sleep 0.05
    done
    cat "$path" "$test_dir/i3-msg.log" >&2
    fail "could not subscribe to i3 window events"
}

assert_no_fullscreen_events() {
    local path="$1"
    if jq -s -e 'any(.[]; .change == "fullscreen_mode")' "$path" >/dev/null; then
        fail "i3 reported a fullscreen transition while the i3 overlay was open"
    fi
}

stop_events() {
    kill "$events_pid" 2>/dev/null || true
    wait "$events_pid" 2>/dev/null || true
    events_pid=
}

start_capture() {
    local desktop="$1"
    local output="$2"
    XDG_CURRENT_DESKTOP="$desktop" "$test_dir/bin/flameshot" gui \
        --region 500x400+200+200 --raw >"$output" 2>"$test_dir/flameshot.log" &
    capture_pid=$!
}

wait_for_capture() {
    local expected="$1"
    local status=0
    wait "$capture_pid" || status=$?
    capture_pid=
    if [[ "$status" != "$expected" ]]; then
        cat "$test_dir/flameshot.log" >&2
        fail "Flameshot exited with status $status; expected $expected"
    fi
}

check_i3_overlay() {
    local expected_mode="$1"
    local window="$2"
    assert_mode "$expected_mode"
    has_flameshot_container &&
        fail "i3 manages the capture overlay instead of bypassing window management"
    local window_info
    window_info="$(xwininfo -id "$window" -all)" ||
        fail "could not inspect the capture overlay"
    [[ "$window_info" == *'Override Redirect State: yes'* ]] ||
        fail "capture overlay is not override-redirect"
    sleep 0.15
    assert_mode "$expected_mode"
}

run_i3_case() {
    local mode="$1"
    local desktop="$2"
    local prefix="$3"
    local fullscreen_command='fullscreen enable'
    local window

    [[ "$mode" == 2 ]] && fullscreen_command='fullscreen enable global'
    i3-msg fullscreen disable >/dev/null
    i3-msg "$fullscreen_command" >/dev/null
    assert_mode "$mode"

    start_events "$test_dir/$prefix-events.json"
    start_capture "$desktop" "$test_dir/$prefix-cancel.png"
    window="$(wait_for_flameshot_window)"
    check_i3_overlay "$mode" "$window"
    xdotool key Escape
    wait_for_capture 2
    [[ ! -s "$test_dir/$prefix-cancel.png" ]] ||
        fail "cancelled capture produced image data"
    assert_mode "$mode"
    assert_no_fullscreen_events "$test_dir/$prefix-events.json"
    stop_events

    start_events "$test_dir/$prefix-capture-events.json"
    start_capture "$desktop" "$test_dir/$prefix-capture.png"
    window="$(wait_for_flameshot_window)"
    check_i3_overlay "$mode" "$window"
    xdotool key r
    xdotool mousemove 250 250 mousedown 1 sleep 0.1 \
        mousemove 450 450 sleep 0.1 mouseup 1
    xdotool key Return
    wait_for_capture 0
    [[ "$(file --brief --mime-type "$test_dir/$prefix-capture.png")" == image/png ]] ||
        fail "capture did not produce a valid PNG"
    assert_mode "$mode"
    assert_no_fullscreen_events "$test_dir/$prefix-capture-events.json"
    stop_events
    printf 'fullscreen mode %s (%s): pass\n' "$mode" "$desktop"
}

run_i3_case 1 'GNOME:i3' 'local'
run_i3_case 2 'i3' 'global'

i3-msg fullscreen disable >/dev/null
i3-msg fullscreen enable >/dev/null
start_events "$test_dir/other-desktop-events.json"
start_capture xfce "$test_dir/other-desktop-cancel.png"
overlay_window="$(wait_for_flameshot_window)"
has_flameshot_container ||
    fail "non-i3 X11 control unexpectedly bypassed window management"
window_info="$(xwininfo -id "$overlay_window" -all)" ||
    fail "could not inspect the non-i3 capture overlay"
[[ "$window_info" == *'Override Redirect State: no'* ]] ||
    fail "non-i3 X11 control is not window-manager managed"
xdotool key Escape
wait_for_capture 2
stop_events

printf 'non-i3 X11 control: pass\n'
