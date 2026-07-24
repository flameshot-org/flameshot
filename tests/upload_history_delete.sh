#!/usr/bin/env sh

# Tests the delete contract of the upload history: deleting a screenshot must
# remove the remote file AND the local history entry, from BOTH delete controls
# (the post-upload dialog's "Delete image" and a history row's delete button).
# Arguments:
# 1. path to tested flameshot executable

# Dependencies:
# - a running desktop session and flameshot daemon
# - "Copy URL after upload" enabled in the configuration; without it the
#   post-upload dialog never appears and there is no dialog delete control
# - for the Google Drive cases: a build with ENABLE_GDRIVE=ON, Drive selected as
#   the upload destination, an OAuth client ID/secret entered in the settings,
#   and a real Google account
# - for the Imgur cases: a build with ENABLE_IMGUR=ON and Imgur selected as the
#   upload destination
# - a way to take the network down for one case (disable Wi-Fi, unplug, or
#   block the host in your firewall)

# HOW TO USE:
# - Start the script with path to tested flameshot executable as the first
#   argument.
#
# - Each case prints what you should expect BEFORE it runs. Read it, run the
#   case, then judge the app against that expectation and record PASS or FAIL.
#
# - The defect these cases exist for is invisible in the app the moment you
#   delete: the remote file goes away and the dialog looks satisfied. It shows
#   up only when the history window is opened AFTERWARDS. So "reopen the
#   history window" in a case is not ceremony — it IS the check.
#
# - The history window builds itself from the cache directory when it OPENS and
#   never reloads. Always CLOSE it and open it again; a window you left open is
#   showing you a stale listing, not a result.
#
# - Cases needing a backend this build does not have, or an account you do not
#   have, are marked. Record those as NOT EXERCISED. Do not record them as PASS.

FLAMESHOT="$1"
[ -z "$FLAMESHOT" ] && FLAMESHOT="flameshot"

if [ -n "$XDG_CACHE_HOME" ]; then
    HISTORY_DIR="$XDG_CACHE_HOME/flameshot/history"
else
    HISTORY_DIR="$HOME/.cache/flameshot/history"
fi

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

# Print a case header for a case needing a specific backend or account.
# $1 = what it needs, rest as in expect()
expect_with() {
    echo
    echo "=============================================================="
    echo "CASE (NEEDS $1): $2"
    echo "EXPECT: $3"
    echo
    echo "If you do not have $1, record this case as NOT EXERCISED and skip it."
    echo "=============================================================="
    wait_for_key
}

# List the cached history entries. This is the ground truth the history window
# only reflects: watch an entry appear on upload and disappear on delete.
show_history() {
    echo
    echo "--- cached history entries in $HISTORY_DIR ---"
    if [ -d "$HISTORY_DIR" ]; then
        ls -1t "$HISTORY_DIR" 2>/dev/null || echo "(unreadable)"
        echo "(count: $(ls -1 "$HISTORY_DIR" 2>/dev/null | wc -l))"
    else
        echo "(no history directory yet)"
    fi
    echo "----------------------------------"
}

echo "Upload history delete walkthrough"
echo "Binary under test: $FLAMESHOT"
echo "History cache: $HISTORY_DIR"
echo
echo "Record a verdict for every case: PASS, FAIL, or NOT EXERCISED."
echo
echo "Open the flameshot configuration now and make sure 'Copy URL after"
echo "upload' is ENABLED. With it off, the post-upload dialog never appears and"
echo "half of these cases have no control to click."
wait_for_key

show_history
echo "EXPECT above: whatever you already had. Note the count — the cases below"
echo "are judged on how it changes."
wait_for_key

#   Deleting from the post-upload dialog
# ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛

echo
echo ">>> SECTION 1: deleting from the post-upload dialog"

expect_with "A GOOGLE DRIVE BUILD AND ACCOUNT" \
  "a Drive dialog delete removes the history entry — the reported bug" \
  "Select Google Drive as the upload destination. Capture and upload once, and
  wait for the post-upload dialog to appear.

  Open the history window (tray menu) and confirm the new screenshot is listed,
  then CLOSE it again.

  Now press 'Delete image' in the post-upload dialog and wait a moment for
  Drive to answer. Then open the history window again.

  The screenshot must be ABSENT from the history window, and gone from Drive
  (check drive.google.com). A row that is still listed is the reported bug and
  a FAIL — even though the file really did disappear from Drive."

show_history
echo "EXPECT above: the entry you just deleted is gone, and the count dropped"
echo "by exactly one. A 'gdrive-...' entry still listed here is the defect at"
echo "its source: the history window is only reporting what is on disk."
wait_for_key

expect_with "AN IMGUR BUILD" \
  "an Imgur dialog delete removes the history entry" \
  "Select Imgur as the upload destination. Capture and upload once, confirm the
  screenshot appears in the history window, and CLOSE that window.

  Press 'Delete image' in the post-upload dialog.

  A browser tab opens on Imgur's delete page, and the entry is removed from
  history immediately — WITHOUT waiting for you to confirm in the browser.
  That is the accepted Imgur behavior, not a FAIL: Imgur's delete is a browser
  hand-off and reports success on opening the page.

  Then open the history window again: the screenshot must be absent."

show_history
echo "EXPECT above: the 'imgur-...' entry is gone and the count dropped by one."
wait_for_key

expect_with "A GOOGLE DRIVE BUILD AND ACCOUNT" \
  "a file already deleted in Drive is treated as deleted here too" \
  "Upload once to Drive. Leave the post-upload dialog open, go to
  drive.google.com and delete that file by hand (and empty the trash if you
  want to be thorough).

  Now press 'Delete image' in the dialog.

  NO failure message appears, and the history entry is removed: an
  already-gone remote file counts as deleted. A failure message here is a
  FAIL — it would leave the user with a permanent history entry they cannot
  get rid of from this dialog."

#   Deleting from a history row
# ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛

echo
echo ">>> SECTION 2: deleting from a history row (must be unchanged)"
echo
echo "These cases worked before this change and must still work: both controls"
echo "now share one removal path, so a regression here is the cost of that."

expect_with "A GOOGLE DRIVE BUILD AND ACCOUNT" \
  "a Drive row delete removes the row and the cached entry" \
  "Upload once to Drive, then open the history window and press the delete
  button on that screenshot's row (confirm if you are asked to).

  The row disappears from the OPEN window straight away — no reopening needed,
  unlike the dialog control — and the file is gone from Drive."

show_history
echo "EXPECT above: the entry is gone and the count dropped by one."
wait_for_key

expect_with "AN IMGUR BUILD" \
  "an Imgur row delete removes the row and the cached entry" \
  "Upload once to Imgur, then delete that screenshot from its history row.

  A browser tab opens on Imgur's delete page and the row disappears from the
  open window immediately."

show_history
echo "EXPECT above: the entry is gone and the count dropped by one."
wait_for_key

expect "a row whose cached file vanished behind the app's back" \
  "With the history window CLOSED, delete one thumbnail file directly from
  $HISTORY_DIR with your file manager or rm. Then open the history window.

  The vanished entry is simply not listed (the window lists the directory), and
  deleting any OTHER row still works normally. No crash, no error dialog."

expect "a legacy entry with no type or token still deletes cleanly" \
  "Legacy entries predate the packed 'type-token-file' name: their filename has
  no '-' segments at all. Create one by copying an existing thumbnail to a bare
  name, e.g.:

    cp $HISTORY_DIR/<some-entry>.png $HISTORY_DIR/legacyentry.png

  Open the history window: 'legacyentry.png' is listed. Delete it from its row.

  The row disappears and legacyentry.png is gone from the cache directory. The
  remote delete itself may well fail or be meaningless for a fabricated entry —
  what is under test is that the RIGHT cached file was resolved and removed,
  not some other row's file."

show_history
echo "EXPECT above: legacyentry.png is gone and no OTHER entry disappeared with"
echo "it. A second entry vanishing means the name resolved to the wrong file."
wait_for_key

#   Failure paths
# ┗━━━━━━━━━━━━━━┛

echo
echo ">>> SECTION 3: a failed delete keeps the entry and says why"
echo
echo "This is the half of the contract that inspection cannot confirm. Both"
echo "causes below were silent before this change: the dialog did nothing"
echo "visible at all, which looks exactly like the reported bug."

expect_with "A GOOGLE DRIVE BUILD AND ACCOUNT" \
  "an offline dialog delete reports the failure and keeps the entry" \
  "Upload once to Drive and leave the post-upload dialog open. NOW take the
  network down (disable Wi-Fi / unplug / firewall-block).

  Press 'Delete image'.

  A failure message appears in the dialog naming the delete as the thing that
  failed. Silence is a FAIL.

  Bring the network back, then open the history window: the screenshot is STILL
  LISTED. That is deliberate — a delete that failed keeps a recoverable entry,
  so you can retry rather than lose the record of a file that still exists."

show_history
echo "EXPECT above: the entry is still present. Its disappearance would be a"
echo "FAIL: it would mean the local entry was dropped for a remote file that"
echo "was never deleted."
wait_for_key

expect_with "A GOOGLE DRIVE BUILD AND ACCOUNT" \
  "canceling Google consent during a delete reports the failure" \
  "Upload once to Drive and leave the post-upload dialog open. Open the
  configuration and press 'Disconnect' under Google Drive, so the next Drive
  request needs fresh consent. Close the configuration.

  Press 'Delete image'. A browser tab opens asking for consent — CLOSE it or
  deny it instead of granting.

  A failure message appears in the dialog saying authorization was canceled and
  the file was not deleted. This path said NOTHING before this change, which is
  precisely why it is a case: no message is a FAIL.

  Open the history window: the screenshot is still listed."

expect_with "A GOOGLE DRIVE BUILD AND ACCOUNT" \
  "a failure message is shown once, not twice" \
  "Repeat the offline case above and watch the dialog closely.

  Exactly ONE failure message is shown. Two messages stacking, or one message
  immediately replaced by a second identical one, is a FAIL: the backend and
  the dialog would both be reporting the same failure."

#   The in-flight guard
# ┗━━━━━━━━━━━━━━━━━━━━┛

echo
echo ">>> SECTION 4: one delete at a time"

expect_with "A GOOGLE DRIVE BUILD AND ACCOUNT" \
  "a second click cannot start a parallel delete" \
  "Upload once to Drive. Press 'Delete image' and IMMEDIATELY click it again
  two or three more times, faster than Drive can answer.

  The button greys out on the first click and ignores the rest, so exactly one
  delete is issued. Check drive.google.com and the history: one file deleted,
  one entry removed, and no error from a second delete finding the file already
  gone."

expect_with "A GOOGLE DRIVE BUILD AND ACCOUNT" \
  "the control comes back after a failure" \
  "Repeat the offline failure case (network down, press 'Delete image', wait
  for the failure message). Then bring the network back UP.

  The 'Delete image' button is clickable again, and pressing it now succeeds:
  the entry disappears from the history window on the next open. A permanently
  greyed-out button after a failure is a FAIL — the user would be stuck with an
  entry they cannot delete from this dialog."

#   Known limitations — do not record these as failures
# ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛

echo
echo ">>> SECTION 5: accepted limitations (NOT failures)"
echo
echo "The behaviors below are known and deliberate. They are listed so you do"
echo "not spend a verdict on them, and do not report them as regressions."
echo
echo "1. Closing the post-upload dialog before a Drive delete finishes may"
echo "   leave the history entry behind. The dialog owns the network connection"
echo "   it deletes through, so closing it aborts the request and neither"
echo "   outcome is ever reported. The entry surviving is the safe side of that"
echo "   race: a delete with no known outcome keeps its entry. You can still"
echo "   delete the row from the history window afterwards."
echo "   Reproduce it if you like — then record it as EXPECTED, not FAIL."
echo
echo "2. An already-open history window does not refresh. It lists the cache"
echo "   directory when it opens and never again, so it misses new uploads just"
echo "   as it misses deletes. Always close and reopen it."
echo
echo "3. An Imgur entry is removed from history when the browser delete page"
echo "   OPENS, not when you confirm there. Abandoning that page loses the"
echo "   history entry for a file that still exists on Imgur. Imgur offers no"
echo "   way for the app to learn the outcome."
echo
echo "4. 'Copy URL' and 'Open URL' stay enabled after a successful delete and"
echo "   point at a dead link."
wait_for_key

echo
echo "Walkthrough complete. Total the verdicts."
