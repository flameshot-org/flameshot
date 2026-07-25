#!/usr/bin/env sh

# Tests for recipient suggestion in the Google Drive upload dialog: the people
# and group lookups behind the "Specific people by email" field, the chip field
# itself, and what happens when the directory is unavailable.
# Arguments:
# 1. path to tested flameshot executable

# Dependencies:
# - a running desktop session and flameshot daemon
# - a build with ENABLE_GDRIVE=ON, Drive selected as the upload destination, and
#   an OAuth client ID/secret entered in the settings
# - the People API and the Cloud Identity API enabled on the OAuth client's
#   Cloud project. Without them every lookup fails and the whole script reduces
#   to section 5 (degradation)
# - a Google Workspace account, and a tenant that does not restrict directory or
#   group visibility
#
# The account needs these fixtures. Cases that need one you do not have are
# marked, and must be recorded NOT EXERCISED rather than PASS:
#   1. a colleague whose surname you know
#   2. a colleague with an alternate (alias) address, whose primary address you
#      also know
#   3. two directory entries that share a display name
#   4. at least two groups you are a direct member of
#   5. a group you are NOT a member of, whose address you know
#   6. a second Google account, for the account-switch case

# HOW TO USE:
# - Start the script with path to tested flameshot executable as the first
#   argument.
#
# - Each case prints what you should expect BEFORE it runs. Read it, run the
#   case, then judge the app against that expectation and record PASS or FAIL.
#
# - Every case reaches the recipient field the same way: capture, then in the
#   upload confirmation dialog choose "Specific people by email". The script
#   stops saying so after section 1.
#
# - The point of this feature is that a suggestion is FASTER and MORE ACCURATE
#   than typing an address from memory. A case where the suggestion is correct
#   but the field fights you (rows reordering under the cursor, the popup eating
#   a keystroke, the dialog refusing to close) is a FAIL even when the right
#   address ends up being shared.
#
# - Nothing here may ever open a browser window. A consent tab appearing while
#   you type in the recipient field is the worst failure this script can find:
#   record it as FAIL and stop, because it means a lookup reached the consenting
#   token path.

FLAMESHOT="$1"
[ -z "$FLAMESHOT" ] && FLAMESHOT="flameshot"

CONFIG="${XDG_CONFIG_HOME:-$HOME/.config}/flameshot/flameshot.ini"
CONFIG_DIR="${XDG_CONFIG_HOME:-$HOME/.config}/flameshot"
CACHE_DIR="${XDG_CACHE_HOME:-$HOME/.cache}/flameshot"
DAEMON_LOG="/tmp/flameshot_recipient_walkthrough.log"

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

# Print a case header for a case needing a fixture. $1 = fixture, rest as in
# expect()
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

# Search everything flameshot writes for a string that came from the directory.
# $1 = what to look for, $2 = how to describe it
scan_for_trace() {
    if [ -z "$1" ]; then
        echo "(no search term given -- record the trace cases NOT EXERCISED)"
        return
    fi
    echo
    echo "--- searching flameshot's own files for $2 ---"
    echo "config:  $CONFIG_DIR"
    echo "cache:   $CACHE_DIR"
    echo "log:     $DAEMON_LOG"
    found=0
    for target in "$CONFIG_DIR" "$CACHE_DIR" "$DAEMON_LOG"; do
        if [ -e "$target" ]; then
            if grep -ril -- "$1" "$target" 2>/dev/null | grep -q .; then
                echo "FOUND IN:"
                grep -ril -- "$1" "$target" 2>/dev/null
                found=1
            fi
        fi
    done
    if [ "$found" = 0 ]; then
        echo "not found anywhere (this is the expected result)"
    fi
    echo "------------------------------------------------"
}

echo "Google Drive recipient suggestion walkthrough"
echo "Binary under test: $FLAMESHOT"
echo
echo "Record a verdict for every case: PASS, FAIL, or NOT EXERCISED."
echo
echo "Two things to set up first."
echo
echo "1. Restart the daemon with its output captured, so the never-log cases in"
echo "   section 7 have something to search:"
echo
echo "     pkill flameshot; $FLAMESHOT >$DAEMON_LOG 2>&1 &"
echo
echo "2. Open the flameshot configuration and set the Google Drive default"
echo "   sharing to 'Specific people by email', so every capture lands on the"
echo "   recipient field without extra clicks."
wait_for_key

echo
echo "Two fixtures are needed as text, for the searches in section 7."
printf "A colleague's surname you will type as a prefix: "
read COLLEAGUE_SURNAME
printf "That colleague's full email address: "
read COLLEAGUE_ADDRESS
echo
echo "Noted. These are only used to search flameshot's files later; they are"
echo "not sent anywhere by this script."
wait_for_key

#   People suggestions
# ┗━━━━━━━━━━━━━━━━━━━┛

echo
echo ">>> SECTION 1: people from the organization directory"

expect "a surname prefix suggests the colleague (AE1)" \
  "Capture a region. In the upload dialog choose 'Specific people by email',
  then type the first two or three letters of your colleague's surname into the
  recipient field.

  A suggestion list appears below the field within about a second. Your
  colleague is in it, showing BOTH their display name and their email address.

  A list showing names without addresses is a FAIL: two people in this
  organization can share a display name, so a name alone does not identify who
  you are about to share with."

expect "an address prefix suggests the same people (AE2)" \
  "Clear the field and type the first letters of that colleague's email address
  instead of their name.

  The same colleague appears. The prefix matches either field, so the user does
  not have to know which one they are typing."

expect "one character suggests nothing" \
  "Clear the field and type a SINGLE letter -- pick one many colleagues' names
  begin with.

  No suggestion list appears at all. One letter matches most of an organization
  and the dialog opens after every capture, so a single character deliberately
  issues no lookup.

  Then type a second letter: the list appears. If a single letter already
  produced a list, the minimum is not being enforced -- FAIL."

expect "typing onward does not show a stale list" \
  "Clear the field. Type a two-letter prefix, wait for the list, then keep
  typing quickly to a longer prefix that matches something DIFFERENT (or
  nothing).

  The list ends up matching what is in the field NOW. It must never settle back
  to results for the shorter prefix you have already typed past, and rows must
  not visibly reshuffle under the cursor after they appear.

  A list that flickers back to an earlier prefix's results is a FAIL: a reply
  for an abandoned search landed on top of a newer one."

expect_with "A COLLEAGUE WITH AN ALIAS ADDRESS" \
  "an alias finds its owner, and the primary address is what gets used (AE5)" \
  "Clear the field and type the first letters of the colleague's ALTERNATE
  address.

  The colleague appears. Pick them.

  The chip must show their PRIMARY address -- not the alias you typed. An alias
  is how you found them, not how they should be addressed, and the chip is what
  gets shared with.

  A chip carrying the alias is a FAIL even though the share would probably still
  work."

#   Group suggestions
# ┗━━━━━━━━━━━━━━━━━━┛

echo
echo ">>> SECTION 2: groups you belong to"

expect "a team-name prefix suggests a group, alongside people (AE3)" \
  "Type the first letters of the name of a group you are a member of.

  The group appears in the list with its name and address. If people also match
  that prefix, the group is listed FIRST.

  Groups are listed first on purpose: they match locally with no network call,
  so they appear instantly and people arrive after. Rows must not reorder when
  the people reply lands."

expect "a group address prefix works too" \
  "Type the first letters of that group's ADDRESS rather than its name.

  The same group appears."

expect_with "A GROUP YOU ARE NOT A MEMBER OF" \
  "a group you do not belong to is not suggested, but is still shareable (AE4)" \
  "Type the name of that group, then its address.

  Neither produces a suggestion -- only your own direct memberships are
  suggested, which is a documented limit and not a bug.

  Now type that group's address IN FULL and commit it with Enter. It becomes a
  chip. Complete the upload.

  The share reaches the group: the uploader retries a recipient as a group when
  it is not a user. A failure notice naming the group address is a FAIL."

expect "a second capture in the same session is instant" \
  "Without restarting flameshot, capture again and type the same group-name
  prefix.

  The group appears with no perceptible delay -- the membership list is fetched
  once per process, not once per capture, and a dialog is built for every
  capture.

  OPTIONAL, if you can watch network traffic (mitmproxy, tcpdump filtered to
  cloudidentity.googleapis.com): the second capture issues NO group call. Two
  calls for two captures is a FAIL. Without traffic visibility, judge on the
  delay only and say so in your notes."

expect_with "A SECOND GOOGLE ACCOUNT" \
  "switching account offers the new account's groups, never the old ones" \
  "Note one group name that is suggested for the current account. Then open the
  configuration, press 'Disconnect' under Google Drive, close it, capture, and
  consent with the SECOND account.

  Type the noted group's prefix. The FIRST account's group must not appear.
  Type a prefix for a group the second account belongs to: that one does.

  Seeing the previous account's teams is a FAIL: a cached list outlived the
  account it belonged to."

#   The recipient field
# ┗━━━━━━━━━━━━━━━━━━━━┛

echo
echo ">>> SECTION 3: the recipient field itself"

expect "each commit key produces a chip" \
  "In the recipient field, type a full valid address and press Enter. Type
  another and press Tab. Type another and type a comma. Type another and type a
  semicolon.

  Each becomes its own chip, and the typing area stays after the last chip so
  you can keep going.

  Tab must not move focus out of the field while text is pending, and Enter must
  not confirm the upload while text is pending."

expect "pasting a list produces one chip per address" \
  "Copy a comma-separated list of three valid addresses from somewhere else and
  paste it into the field.

  Three chips, in the order pasted. This is how the old field was used, so it
  has to keep working."

expect "a valid address matching nobody is marked, and still uploads (AE6)" \
  "Type a syntactically valid address that belongs to nobody -- misspell your
  colleague's, or use an external one -- and commit it.

  It becomes a chip that is VISIBLY DIFFERENT from a resolved one: marked with a
  warning sign, outlined differently, and carrying a tooltip saying it was not
  found in your organization. The Upload button stays available.

  A chip that looks exactly like a resolved colleague is a FAIL -- being able to
  see a mistake BEFORE the upload is the point. A refusal to upload it is also a
  FAIL: external partners and anything the directory cannot see must stay
  shareable."

expect "text that is not an address is not silently dropped" \
  "Type something that is not an address at all ('notanaddress') and type a
  comma.

  It does NOT become a chip, and it does NOT vanish either: the text stays in
  the field where you can see and fix it.

  Text disappearing on a comma is a FAIL. The old field dropped malformed
  addresses without a word, which is how a typo used to reach an upload."

expect_with "TWO ENTRIES SHARING A DISPLAY NAME" \
  "two people with the same name make distinguishable chips (AE11)" \
  "Type a prefix that matches both entries. Both appear in the list. Add both as
  recipients.

  The two chips are told apart without hovering, clicking, or any further
  interaction -- each carries its own address next to the shared name."

expect "chips can be removed, by key and by mouse" \
  "With several chips present and the typing area empty, press Backspace: the
  LAST chip is removed. Press it again: the one before it goes.

  Then click the small remove affordance on a chip in the middle: that chip goes
  and the others stay.

  With text pending in the input, Backspace must edit the text instead -- it must
  not delete a chip out from under you."

expect "a repeated address is added once" \
  "Add an address, then add the very same address again -- once exactly as
  before, once with different capitalisation.

  No duplicate chip appears either time. The recipients are the ones you can
  see."

#   What reaches the upload
# ┗━━━━━━━━━━━━━━━━━━━━━━━┛

echo
echo ">>> SECTION 4: what reaches the upload"

expect "confirming during a lookup uploads immediately (AE7)" \
  "Type a two-or-three letter prefix and, WITHOUT waiting for the suggestion
  list, immediately press the Upload button.

  The upload starts at once with whatever recipients are in the field. If the
  prefix you typed was not a valid address, the dialog says so and names it
  instead -- that is also correct. What must NOT happen is the dialog hanging,
  greying out, or waiting for the lookup to finish."

expect "the addresses shared with are the resolved ones" \
  "Add one recipient by picking a suggestion and one by typing an address in
  full, then upload. When it finishes, open the file in Google Drive and look at
  who it is shared with.

  Both people are there, by email address. A display name reaching Drive as if
  it were an address, or a share missing the picked colleague, is a FAIL."

expect "accepting with an empty field still refuses" \
  "With 'Specific people by email' selected and NO chips and NO text, press
  Upload.

  The dialog refuses and asks for at least one recipient, exactly as it did
  before this feature. It must not upload a file shared with nobody while
  reporting success."

expect "switching visibility away and back is not surprising" \
  "Add two recipients. Switch the visibility selector to 'Private (only you)':
  the recipient field and its label disappear. Switch back to 'Specific people
  by email': your two recipients are still there, unchanged.

  Now switch to 'Private' again and upload. The file is private and NOT shared
  with those two people -- recipients only apply to the level that collects
  them."

expect "an Imgur-only build shows no Drive controls" \
  "Run a build compiled with ENABLE_GDRIVE=OFF and ENABLE_IMGUR=ON, and upload.

  The confirmation dialog has no visibility selector and no recipient field at
  all -- just the upload question. The recipient field is shared code, so this
  case is what proves no Drive-only type leaked into it.

  If you have only one build, record this NOT EXERCISED; the build gates in the
  plan cover the compile side."

#   Degradation
# ┗━━━━━━━━━━━━┛

echo
echo ">>> SECTION 5: when lookup is unavailable"
echo "Every case here must feel like the field simply has no suggestions --"
echo "never like something is broken."

expect "offline: no suggestions, no error, upload still works" \
  "Disconnect the machine from the network. Capture, choose 'Specific people by
  email', and type a colleague's surname prefix.

  No suggestion list appears. NO error dialog, NO notification, no red text.
  Type a full address, commit it, and press Upload: the upload fails (there is
  no network) but it fails at the UPLOAD, with an upload message -- not with a
  complaint about suggestions.

  Reconnect afterwards."

expect_with "AN ACCOUNT WHERE THE DIRECTORY SCOPE WAS DECLINED" \
  "declining the directory permission degrades quietly (AE9)" \
  "Press 'Disconnect', capture, and on the consent screen UNCHECK the directory
  and/or group permissions while granting Drive access.

  Type a colleague's surname prefix: no suggestions appear, and no error dialog
  is raised. Type an address in full and upload: it shares normally.

  Then capture twice more. Still no suggestions, still no error, and NO consent
  tab reopens. Re-prompting on every capture is a FAIL.

  If your consent screen does not offer these as optional checkboxes, record
  this NOT EXERCISED."

expect "a tenant that refuses groups still suggests people" \
  "This is the shape to look for rather than something you can force: if your
  tenant restricts group visibility but not directory visibility, typing a
  colleague's surname still suggests PEOPLE while no group ever appears.

  If your tenant allows both, record this NOT EXERCISED -- do not record it as
  PASS on the strength of groups working."

#   Consent
# ┗━━━━━━━━┛

echo
echo ">>> SECTION 6: consent, exactly once"

expect "upgrading an existing grant re-consents once (AE8)" \
  "Quit flameshot completely. Edit $CONFIG by hand: set

      gdriveGrantedScopes=https://www.googleapis.com/auth/drive.file openid email

  (the scope set from before this feature) and leave gdriveRefreshToken alone.
  Restart the daemon as in the setup step, then capture and upload.

  The consent screen opens ONCE, now also asking for directory and group access.
  Approve it. The upload completes.

  Then capture and upload twice more: NO consent tab on either. A prompt on
  every upload is a FAIL -- the requested scope string and the stored marker
  disagree, and users would be nagged forever."

expect "a suggestion lookup never opens a consent window" \
  "Quit flameshot. Edit $CONFIG and blank the refresh token:

      gdriveRefreshToken=

  Restart the daemon, capture, choose 'Specific people by email', and type a
  colleague's surname prefix. Wait several seconds. Type more.

  NO browser window opens, and no consent tab appears. There are simply no
  suggestions.

  A consent tab here is the most serious failure in this script: the suggestion
  path reached the consenting token path, and a user would get a browser window
  in the middle of sharing a screenshot. Record FAIL and stop.

  Then press Upload with a typed address: NOW the consent screen may open, which
  is correct -- the upload path is allowed to ask."

expect "a fresh connection consents once and then stays quiet" \
  "Press 'Disconnect', then capture and upload, granting consent.

  Consent opens once. Capture and upload twice more: no consent tab, and
  suggestions work on both -- the first upload's token is what the lookups then
  reuse."

#   Nothing is kept
# ┗━━━━━━━━━━━━━━━━┛

echo
echo ">>> SECTION 7: nothing about the organization is kept"

expect "look up several colleagues, then restart (AE10)" \
  "Capture and type prefixes matching several colleagues and at least one group,
  picking a few as recipients. Complete one upload. Then QUIT flameshot
  completely and restart it (with output captured, as in the setup step).

  Capture again and open the recipient field WITHOUT typing anything.

  The field is empty and offers nothing: no recently-used recipients, no
  previously-seen colleagues, no history. There is deliberately no memory of
  who you shared with -- the shares that hurt are to people you have never sent
  to before.

  A list of previous recipients appearing before you type is a FAIL."

scan_for_trace "$COLLEAGUE_SURNAME" "the colleague's surname"
echo "EXPECT above: not found. A hit inside the config or cache directory means"
echo "directory content was written to disk, which is a FAIL."
wait_for_key

scan_for_trace "$COLLEAGUE_ADDRESS" "the colleague's address"
echo "EXPECT above: the address may legitimately appear NOWHERE. It is allowed"
echo "in nothing that flameshot writes: not the config, not the history cache,"
echo "and not the log."
echo
echo "One exception to read carefully: an address you typed or picked is sent to"
echo "Google as a share recipient, but it is never stored locally. If you find"
echo "it in $DAEMON_LOG, that is a FAIL -- directory content and typed prefixes"
echo "are never logged."
wait_for_key

echo
echo "--- the captured daemon output, for a last look ---"
if [ -r "$DAEMON_LOG" ]; then
    wc -l "$DAEMON_LOG"
    echo "(searching for anything that looks like a directory lookup)"
    grep -in "searchDirectoryPeople\|searchDirectGroups\|suggestion" \
      "$DAEMON_LOG" 2>/dev/null || echo "nothing about lookups was logged"
else
    echo "(no log captured -- record the log cases NOT EXERCISED)"
fi
echo
echo "EXPECT above: nothing. Not an endpoint, not a prefix, not a name."
wait_for_key

echo
echo "Walkthrough complete. Total the verdicts."
echo
echo "Coverage of the plan's acceptance examples:"
echo "  AE1  section 1, surname prefix"
echo "  AE2  section 1, address prefix"
echo "  AE3  section 2, team-name prefix"
echo "  AE4  section 2, group you do not belong to"
echo "  AE5  section 1, alias finds its owner"
echo "  AE6  section 3, marked chip for an address matching nobody"
echo "  AE7  section 4, confirming during a lookup"
echo "  AE8  section 6, upgrading an existing grant"
echo "  AE9  section 5, declined directory permission"
echo "  AE10 section 7, restart keeps nothing"
echo "  AE11 section 3, two people sharing a display name"
