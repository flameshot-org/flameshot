#!/usr/bin/env sh

# Tests for the capture-completion lifecycle: the hand-off from the capture
# overlay to the export/upload flow, and the upload confirmation dialog.
# Arguments:
# 1. path to tested flameshot executable

# Dependencies:
# - a running desktop session and flameshot daemon
# - for the upload cases: a build with an uploader enabled (ENABLE_GDRIVE and/or
#   ENABLE_IMGUR) and, for Google Drive, a configured account

# HOW TO USE:
# - Start the script with path to tested flameshot executable as the first
#   argument.
#
# - Each case prints what you should expect BEFORE it runs. Read it, run the
#   case, then judge the app against that expectation and record PASS or FAIL.
#
# - Two things to watch on almost every case:
#     1. The app must not crash.
#     2. The capture overlay must be GONE before any export or upload UI (the
#        confirmation dialog, the save dialog) appears. If you can still see the
#        darkened selection overlay behind a dialog, that is a FAIL.
#
# - Some cases are platform-gated (GNOME/Wayland, macOS). If you are not on that
#   platform, record them as NOT EXERCISED. Do not record them as PASS.
#
# - Some cases need settings changed in the flameshot configuration UI. The
#   script tells you which and pauses so you can change them.

FLAMESHOT="$1"
[ -z "$FLAMESHOT" ] && FLAMESHOT="flameshot"

# --raw >/dev/null is a hack that makes the subcommand wait for the daemon to
# finish the pending action
flameshot() {
    command "$FLAMESHOT" "$@" --raw >/tmp/img.png
}

# Print the given command and run it
cmd() {
    echo "$*" >&2
    "$@"
    sleep 1
}

wait_for_key() {
    echo "Press Enter to continue..." >&2 && read ____
}

# Print a case header and the expectation the operator judges against
expect() {
    echo
    echo "=============================================================="
    echo "CASE: $1"
    echo "EXPECT: $2"
    echo "=============================================================="
    wait_for_key
}

# Print a platform-gated case header. $1 = platform, $2 = case, $3 = expectation
expect_on() {
    echo
    echo "=============================================================="
    echo "CASE ($1 ONLY): $2"
    echo "EXPECT: $3"
    echo
    echo "If you are NOT on $1, record this case as NOT EXERCISED and skip it."
    echo "=============================================================="
    wait_for_key
}

echo "Capture-completion lifecycle walkthrough"
echo "Binary under test: $FLAMESHOT"
echo
echo "Record a verdict for every case: PASS, FAIL, or NOT EXERCISED."

#   Upload confirmation dialog
# ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛

echo
echo ">>> SECTION 1: upload confirmation dialog"
echo "These cases need an uploader-enabled build. For the Google Drive cases you"
echo "also need Drive selected as the upload destination; the first case signs the"
echo "account in, and the rest reuse it."
echo "Open the flameshot configuration now and make sure 'Upload without"
echo "confirmation' is DISABLED."
wait_for_key

expect "Drive first-time consent, browser redirect lands" \
  "Run this case FIRST, on a disconnected account: open the flameshot
  configuration, press 'Disconnect' under Google Drive, and close it. Then make
  a selection and choose the upload tool, and confirm the dialog.

  The system browser opens Google's consent page. Pick the account and grant
  the permissions. The browser must then land on Flameshot's own page reading
  'Flameshot is now authorized.'

  A browser error instead of that page is a FAIL — ERR_CONNECTION_REFUSED or an
  empty/'reset' response means the loopback listener was torn down with the
  reply still buffered, so the reply never reached the browser and the retry hit
  a closed port. Note that the upload itself can still succeed in that case, so
  judge this case on the BROWSER page, not on the upload result.

  Leave the account connected — the cases below need it."
cmd command "$FLAMESHOT" gui

expect "Drive upload, confirm" \
  "Make a selection and choose the upload tool. The overlay disappears, THEN
  the confirmation dialog appears. Confirm it. The upload proceeds and the app
  does not crash. (This is the reported crash; it must not reproduce.)"
cmd command "$FLAMESHOT" gui

expect "Drive upload, each non-default visibility" \
  "Repeat the upload three times, choosing a different sharing visibility each
  time: 'Private (only you)', 'Anyone on the internet', and 'Specific people'.
  Each upload must use the visibility you picked. Check the file's sharing
  settings in Drive afterwards — an empty or default visibility where you chose
  something else is a FAIL."
cmd command "$FLAMESHOT" gui
wait_for_key
cmd command "$FLAMESHOT" gui
wait_for_key
cmd command "$FLAMESHOT" gui

expect "Drive upload, specific people by email" \
  "Choose 'Specific people by email' and enter one or more valid addresses.
  Confirm. The upload happens and those recipients are granted access on the
  Drive file."
cmd command "$FLAMESHOT" gui

expect "Drive upload, specific people with no valid address" \
  "Choose 'Specific people by email' and leave the field empty (or type a
  non-address). Confirm. The dialog STAYS OPEN showing a validation message. No
  upload happens, nothing crashes. Cancel out when done."
cmd command "$FLAMESHOT" gui

expect "Drive upload, decline the public-share warning" \
  "Choose 'Anyone on the internet' and confirm, then DECLINE the warning about
  sharing publicly. The dialog stays open and no upload happens. Cancel out."
cmd command "$FLAMESHOT" gui

expect "Reject the confirmation dialog" \
  "Make a selection, choose the upload tool, then reject/cancel the confirmation
  dialog. No upload happens, the app does not crash, and the capture overlay is
  gone — no leftover overlay on screen."
cmd command "$FLAMESHOT" gui

echo
echo "Now open the flameshot configuration and ENABLE 'Upload without"
echo "confirmation' for the next case."
wait_for_key

expect "Upload without confirmation" \
  "Make a selection and choose the upload tool. NO confirmation dialog appears
  and the upload proceeds directly."
cmd command "$FLAMESHOT" gui

echo
echo "Restore 'Upload without confirmation' to DISABLED before continuing."
wait_for_key

expect_on "IMGUR-ENABLED BUILD WITH DRIVE DISABLED" \
  "Imgur upload regression" \
  "The confirmation dialog shows NO visibility selector. Confirming uploads to
  Imgur; the URL is copied if that option is on, and the entry appears in the
  upload history. Deleting from history still works."
cmd command "$FLAMESHOT" gui

#   The R8 task matrix
# ┗━━━━━━━━━━━━━━━━━━━━┛

echo
echo ">>> SECTION 2: every final-action task behaves as before"

expect "SAVE via the save dialog" \
  "Make a selection and choose save. The overlay disappears BEFORE the file
  dialog opens. Save the file and confirm it is written and the notification
  appears."
cmd command "$FLAMESHOT" gui

expect "SAVE to a given path" \
  "Make a selection and accept. The file is written under /tmp/ and the
  notification appears."
cmd flameshot gui --path /tmp/

expect "COPY to clipboard" \
  "Make a selection and accept. The image is on the clipboard — paste it
  somewhere to confirm."
cmd flameshot gui --clipboard

expect "PIN to screen" \
  "Make a selection and accept. The selection is pinned to the screen. Close the
  pin afterwards."
cmd flameshot gui --pin

expect "PRINT geometry" \
  "Make a selection and accept. The selection geometry is printed to stdout as
  WxH+X+Y."
cmd command "$FLAMESHOT" gui --print-geometry

expect "Accept on select" \
  "Make a selection. The capture completes as soon as the selection settles,
  without you accepting. The overlay does not linger and the export runs."
cmd flameshot gui --clipboard --accept-on-select

expect "Accept on select with a preset region" \
  "No overlay appears at all and no selection is asked for: the region is
  already set, so the capture completes immediately and the image lands on the
  clipboard. Paste it to confirm. A silent no-op here — nothing copied, no
  notification — means the completed capture was dropped instead of exported."
cmd flameshot gui --clipboard --accept-on-select --region 400x300+0+0

expect "Accept on select with the last region" \
  "Same as the previous case, using the remembered region instead of an
  explicit one. The capture completes with no overlay and the image is copied."
cmd flameshot gui --clipboard --accept-on-select --last-region

#   Cancellation and failure
# ┗━━━━━━━━━━━━━━━━━━━━━━━━━━┛

echo
echo ">>> SECTION 3: cancellation and failure paths"

expect "Cancel with Esc" \
  "Press Esc without accepting. 'Screenshot aborted' is reported and the overlay
  closes. Nothing is saved, copied, or uploaded."
cmd command "$FLAMESHOT" gui

expect "Cancel with the exit tool" \
  "Choose the exit/cancel tool in the overlay. 'Screenshot aborted' is reported
  and the overlay closes."
cmd command "$FLAMESHOT" gui

echo
echo "CASE: CLI exit code on cancellation"
echo "EXPECT: cancel the capture with Esc; the command exits non-zero"
echo "        (E_ABORTED = 2). The exit code is printed below."
wait_for_key
command "$FLAMESHOT" gui --path /tmp/
echo ">> exit code was: $? (expected 2)"

expect_on "GNOME/WAYLAND" \
  "Clipboard workaround keeps the window alive" \
  "Make a selection and accept with a COPY task. The capture window stays alive
  until the compositor fetches the clipboard data, then closes on its own. The
  clipboard receives the image — paste it somewhere to confirm. The log mentions
  keeping the window alive until clipboard data is fetched."
cmd flameshot gui --clipboard

expect_on "GNOME/WAYLAND" \
  "Clipboard workaround combined with a second task" \
  "Make a selection and accept, with COPY *and* SAVE both requested. The
  clipboard gets the image and the save dialog appears ONCE. Leave the save
  dialog sitting open for several seconds before saving. A second save dialog
  appearing behind the first, or the file being written twice, means the
  capture was handed off to the export more than once — the workaround's
  safety-net timer re-closing an already-completed window."
cmd flameshot gui --clipboard --path /tmp/

expect_on "GNOME/WAYLAND" \
  "Clipboard workaround combined with an upload" \
  "Same as above with COPY and upload: choose the upload tool with the
  confirmation dialog enabled, and leave the dialog open for several seconds.
  Exactly one confirmation dialog appears and exactly one upload happens."
cmd flameshot gui --clipboard

expect_on "MACOS" \
  "Capture window replacement" \
  "With a capture overlay already open, request another capture from the tray.
  The existing window is replaced and the app does not crash."
wait_for_key

expect_on "MACOS" \
  "Save dialog finds the capture widget" \
  "Make a selection and choose save. The native save dialog opens correctly and
  the file is written."
cmd command "$FLAMESHOT" gui

echo
echo "CASE (REQUIRES A FORCED FAILURE): screen grab fails at construction"
echo "EXPECT: with the screen grab forced to fail, no capture overlay appears,"
echo "        'Screenshot aborted' is reported, a CLI-invoked capture exits"
echo "        non-zero, and the NEXT capture request still works — the"
echo "        single-active-window guard is not stuck."
echo
echo "This case cannot be triggered from the CLI. Reach it either by running"
echo "where no screen is available, or by temporarily forcing the failure branch"
echo "in ScreenGrabber. If you cannot, record it as NOT EXERCISED."
wait_for_key

expect "Tray capture still returns a usable window" \
  "Take a capture from the tray icon. It behaves normally. On a build with the
  update checker enabled and an update available, the update notification still
  appears over the capture overlay."
wait_for_key

echo
echo '>> All cases done. Make sure every case has a recorded verdict of'
echo '>> PASS, FAIL, or NOT EXERCISED.'
