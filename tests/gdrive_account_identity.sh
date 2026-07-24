#!/usr/bin/env sh

# Tests for the Google Drive account identity: the account email and the
# organization domain that "anyone in your organization" sharing depends on.
# Arguments:
# 1. path to tested flameshot executable

# Dependencies:
# - a running desktop session and flameshot daemon
# - a build with ENABLE_GDRIVE=ON, Drive selected as the upload destination, and
#   an OAuth client ID/secret entered in the settings
# - a Google Workspace account (an organization one). The last section
#   additionally needs a personal @gmail.com account, and one case needs an
#   account whose consent screen offers sign-in as an optional checkbox.

# HOW TO USE:
# - Start the script with path to tested flameshot executable as the first
#   argument.
#
# - Each case prints what you should expect BEFORE it runs. Read it, run the
#   case, then judge the app against that expectation and record PASS or FAIL.
#
# - The whole point of these cases is that the identity is fetched at CONSENT
#   time and cached. A case that says "disconnect first" is not optional
#   ceremony: without it you are re-testing the cache, not the fetch.
#
# - Cases that need a second Google account are marked. If you do not have one,
#   record them as NOT EXERCISED. Do not record them as PASS.

FLAMESHOT="$1"
[ -z "$FLAMESHOT" ] && FLAMESHOT="flameshot"

CONFIG="${XDG_CONFIG_HOME:-$HOME/.config}/flameshot/flameshot.ini"

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

# Print a case header for a case needing a second account. $1 = account, rest
# as in expect()
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

# Show the cached identity keys. Never prints the refresh token.
show_identity() {
    echo
    echo "--- cached identity in $CONFIG ---"
    if [ -r "$CONFIG" ]; then
        grep -E '^gdrive(AccountEmail|AccountDomain|GrantedScopes)=' "$CONFIG" \
          || echo "(none of the three keys present)"
    else
        echo "(config file not readable at $CONFIG)"
    fi
    echo "----------------------------------"
}

echo "Google Drive account identity walkthrough"
echo "Binary under test: $FLAMESHOT"
echo
echo "Record a verdict for every case: PASS, FAIL, or NOT EXERCISED."
echo
echo "Open the flameshot configuration now and set the Google Drive default"
echo "sharing to 'Anyone in your organization with the link', so the domain is"
echo "actually exercised on every upload."
wait_for_key

#   Identity captured at consent
# ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛

echo
echo ">>> SECTION 1: the identity is captured at consent"

expect "Workspace consent populates email AND domain" \
  "Press 'Disconnect' under Google Drive in the configuration, close it, then
  capture and upload once. Grant consent in the browser.

  Google's consent screen must now ask for your name/email alongside Drive
  access. Approve it.

  Then: the upload succeeds, and NO sharing warning appears — the file is
  shared with your organization. Reopen the configuration: the Google Drive
  group must read 'Connected as <your address>', NOT 'Not connected'.

  'Not connected' after a successful upload is the exact regression these cases
  exist for: the token persisted but the identity did not. That is a FAIL."

show_identity
echo "EXPECT above: gdriveAccountEmail is your address, gdriveAccountDomain is"
echo "your organization domain (e.g. example.com, with no '@'), and"
echo "gdriveGrantedScopes lists drive.file, openid and email."
echo
echo "An empty or absent gdriveAccountDomain here is a FAIL even if the upload"
echo "itself succeeded."
wait_for_key

expect "the domain is the Workspace domain, not a guess" \
  "Look at gdriveAccountDomain printed above.

  For an account whose primary domain differs from the mail domain (a Workspace
  with a domain alias), this must be the organization's real primary domain, as
  Google reports it — not merely the text after '@' in your address.

  If your account has no alias, record this as NOT EXERCISED rather than PASS."

expect "later uploads reuse the cached identity silently" \
  "Without disconnecting, capture and upload twice more.

  Both uploads share with the organization and show no warning, and NO browser
  tab opens — the stored grant already carries the sign-in scopes, so there is
  nothing to re-consent."

#   Re-consent on a scope change
# ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛

echo
echo ">>> SECTION 2: upgrading from a grant without the sign-in scopes"
echo "This simulates an account connected by an older Flameshot build, which is"
echo "how the org-domain failure reached users in the first place."

expect "a stale-scope grant re-consents exactly once" \
  "Quit flameshot completely. Edit $CONFIG by hand: set

      gdriveGrantedScopes=https://www.googleapis.com/auth/drive.file

  (i.e. the old Drive-only scope set) and leave gdriveRefreshToken alone. Blank
  out gdriveAccountEmail and gdriveAccountDomain too, matching an old grant that
  never learned them. Restart flameshot, then capture and upload.

  The consent screen must open ONCE, even though a refresh token is stored —
  refreshing can never add a scope, so re-consent is the only way to recover.
  Approve it: the upload then shares with the organization, and the settings
  read 'Connected as ...' again.

  Silently uploading with the org warning and no consent prompt is a FAIL: that
  is the stuck state, and it would never self-heal."

expect "the re-consent does not repeat" \
  "Capture and upload twice more, without touching the config.

  No browser tab opens on either upload. A consent prompt on every upload is a
  FAIL — the grant marker was not updated and users would be nagged forever."

#   Accounts with no organization
# ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛

echo
echo ">>> SECTION 3: accounts and choices that legitimately have no domain"
echo "These must degrade with an accurate message. The bug being guarded against"
echo "is a misleading one — reporting a lookup failure for a situation that is"
echo "not a failure at all."

expect_with "A PERSONAL @gmail.com ACCOUNT" \
  "a personal account reports no organization" \
  "Disconnect, then upload and consent with a personal @gmail.com account.

  The upload succeeds and the file stays private, with a warning saying the
  account is NOT PART OF AN ORGANIZATION. The settings still read
  'Connected as <address>' — the account is known, it simply has no domain.

  A warning claiming the domain could not be READ or DETERMINED is a FAIL: it
  sends the user hunting for a misconfiguration that does not exist.

  Then switch the default sharing to 'Private', 'Specific people' and 'Anyone
  with the link' in turn and upload once for each. All three must work normally
  with no warning — only organization sharing is affected."

expect_with "AN ACCOUNT WITH OPTIONAL SIGN-IN CONSENT" \
  "declining the sign-in permission degrades without nagging" \
  "Disconnect, upload, and on the consent screen UNCHECK the name/email
  permission while granting Drive access.

  The upload still succeeds and the file stays private, with a warning that says
  the SIGN-IN PERMISSION WAS DECLINED and points at reconnecting the account.

  Then upload twice more: the warning repeats, but NO consent tab reopens.
  Re-prompting on every upload is a FAIL."

expect "reconnecting after declining recovers" \
  "Still on the declined account: press 'Disconnect', then upload again and this
  time GRANT the sign-in permission.

  Organization sharing works and the warning is gone — the decline is
  recoverable through the documented route."

#   Disconnect
# ┗━━━━━━━━━━━━┛

echo
echo ">>> SECTION 4: disconnect clears the identity"

expect "disconnect clears email, domain and the grant marker" \
  "Press 'Disconnect' under Google Drive, then close the configuration."

show_identity
echo "EXPECT above: gdriveAccountEmail, gdriveAccountDomain and"
echo "gdriveGrantedScopes are all empty, and gdriveRefreshToken is gone too."
echo
echo "A leftover gdriveGrantedScopes is a FAIL: it would claim the next stored"
echo "token already carries scopes it was never granted."
wait_for_key

echo
echo "Walkthrough complete. Total the verdicts."
