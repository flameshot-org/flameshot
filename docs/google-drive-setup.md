# Google Drive upload — setup guide

Flameshot can upload captures to Google Drive as an alternative to Imgur,
for organizations that want screenshots to stay inside their Google Workspace
domain. This is an opt-in build feature (`-DENABLE_GDRIVE=ON`) that uses an
OAuth client **you register in your own Google Cloud project** — nothing is
shipped in the Flameshot binary.

## What you get

- Uploads go into a single folder named **"Flameshot screenshots"** in the
  signed-in user's own My Drive.
- Flameshot's write access is confined by the least-privilege `drive.file` scope:
  it can only see and touch files it created. It cannot list or open anything
  else in your Drive, and has no access to shared/Team Drives. Everything else it
  is granted is read-only: your email address and organization domain (`openid`,
  `email`), to label the connected account and scope organization sharing, and
  directory lookups for recipient suggestion.
- Per-upload sharing visibility, with a configurable default:
  - Anyone in your organization with the link (the shipped default)
  - Private (only you)
  - Specific people by email (individuals or groups)
  - Anyone on the internet with the link
- Recipient suggestion on the "specific people" level: typing a name, an address
  fragment, or a team name suggests real people from your organization's
  directory and the groups you belong to, so nobody has to remember addresses.
  This uses two additional read-only scopes and is described in
  [Recipient suggestion](#recipient-suggestion) below.

## For the administrator: register the OAuth client

You need a Google Cloud project and an OAuth client of type **Desktop app**.
This is a one-time setup, external to Flameshot.

1. Open the [Google Cloud console](https://console.cloud.google.com/) and
   create a project (or reuse one) inside your Workspace organization.
2. Enable these APIs for that project
   (*APIs & Services → Library → … → Enable*):
   - **Google Drive API** — the uploads themselves.
   - **People API** — suggesting people from your organization's directory.
   - **Cloud Identity API** — suggesting the groups a user belongs to.

   Only the Drive API is required for uploading. Without the other two,
   recipient suggestion finds nothing and the recipient field behaves like a
   plain address field; nothing else breaks.
3. Configure the **OAuth consent screen**:
   - Choose **Internal** user type. An Internal Workspace app skips Google's
     verification entirely and is not subject to the 7-day testing-mode
     refresh-token expiry.
   - Add these scopes:

     | Scope | What it is for | Classification |
     |---|---|---|
     | `.../auth/drive.file` | Upload and share the capture; app-created files only | Non-sensitive |
     | `openid`, `email` | The signed-in address and the account's Workspace domain | Non-sensitive |
     | `.../auth/directory.readonly` | Suggest people from the organization directory | **Sensitive** |
     | `.../auth/cloud-identity.groups.readonly` | Suggest the user's own group memberships | Non-sensitive |

     `directory.readonly` is classified **sensitive**. For an **Internal**-only
     app that is not a verification blocker — Internal apps skip Google's
     verification — but it is a real change from the earlier all-non-sensitive
     scope set, so check it against your organization's app-access policy
     **before** rolling the build out.

     That check matters more than it looks. Flameshot requests all of its scopes
     together in one authorization, so an app-access policy that refuses the
     sensitive scope can refuse the whole grant and take uploading down with it —
     not merely disable suggestions. Two milder cases degrade gracefully instead:
     leaving the People API disabled makes lookups return nothing, and a user
     declining the directory permission at a granular consent screen leaves them
     with a plain address field. Both keep uploads and all four sharing levels
     working.

     The two sign-in scopes are what let Flameshot read the signed-in address and
     the account's Workspace domain, which the "anyone in your organization"
     sharing level needs; `drive.file` alone cannot supply them. Both directory
     scopes are read-only and cannot modify anything.
4. Create credentials → **OAuth client ID** → application type
   **Desktop app**. Google issues a **client ID** and a **client secret**.
   Both are required for Desktop-type clients even though the flow also uses
   PKCE.
5. Give the client ID and client secret to your users (or distribute them
   through your normal configuration channel).

Both values are **non-confidential** under Google's desktop-app model and the
Internal-only consent screen: they are not the secret to protect (see below).

## For the user: connect Flameshot

1. Open Flameshot **Configuration → General**.
2. Set **Upload service** to **Google Drive**.
3. In the **Google Drive** group, paste the **OAuth Client ID** and
   **OAuth Client Secret** provided by your admin.
4. Optionally pick a **Default sharing** level.
5. Trigger an upload. The first time, your system browser opens Google's
   consent screen; approve it and the capture uploads. Later uploads skip
   consent until you disconnect or the token is revoked.

The **Google Drive** settings group shows the connected account and offers a
**Disconnect** button, which clears the stored credentials (and revokes the
refresh token server-side); the next upload re-initiates consent.

## Recipient suggestion

With **Specific people by email** selected, typing in the recipient field
suggests people from your organization's directory — matched on display name or
address, including alternate addresses — alongside the groups you belong to.
Picking a suggestion adds a chip carrying the person's name and address; the
address used is always the one your directory marks primary, not an alias you
may have typed to find them.

An address that matches nobody becomes a visibly different chip carrying a
warning, and is still shared with. That is deliberate: external partners and
anything your directory cannot see must stay shareable, and a misspelling is
worth seeing *before* the upload rather than as a failure afterwards.

Nothing about the organization is written to disk. Lookup results are held in
memory for as long as Flameshot is running and are gone when it exits — there is
no local roster and no recent-recipients list.

### What is not suggested

These all remain shareable by typing the address in full; they simply do not
appear as suggestions.

- **Groups you are not a member of.** Suggestions cover your own direct
  memberships. An organization-wide group search needs a customer identifier an
  ordinary user cannot obtain without an administrator.
- **Groups you reach only through another group.** Transitive membership search
  is restricted to Workspace editions many organizations do not hold.
- **Groups referred to by an alias.** A membership response carries one address
  per group. (Alternate addresses *are* matched for people.)
- **Anyone your tenant hides.** If your Workspace admin settings restrict
  directory contact sharing or group visibility, lookups return nothing. The
  recipient field then behaves like the plain address field it replaced, with no
  error and no blocked upload.

If a user reports any of these as a bug, it is this list.

### Upgrading from a build before recipient suggestion

Recipient suggestion adds two scopes, and Google never widens an existing
authorization on refresh. An account connected by an earlier build therefore
meets the consent screen **once** on its next upload, now also asking for
directory and group access; approve it and later uploads are silent again. Until
then, uploads work normally and the recipient field simply offers no suggestions.

If your consent screen offers the directory permissions as optional checkboxes
and you decline them, Flameshot keeps working with no suggestions and will not
re-prompt on every upload. Reconnect the account to change that decision.

Typing in the recipient field never opens a consent window on its own. If a
browser tab appears while you are typing a recipient, that is a bug worth
reporting — only an upload is allowed to ask for consent.

### Upgrading from a build before the sign-in scopes

Google never widens an existing authorization on refresh, so an account
connected by an earlier Flameshot build holds a `drive.file`-only grant that can
never learn your organization domain. The first upload after upgrading opens the
consent screen once to pick up the added scopes; approve it and later uploads go
back to being silent. Until then, "anyone in your organization" uploads fall
back to private with a warning.

If your consent screen offers the sign-in permission as an optional checkbox and
you decline it, Flameshot keeps working but organization sharing stays
unavailable — it will not re-prompt you on every upload. Reconnect the account
to change that decision.

A personal (non-Workspace) Google account has no organization domain at all.
Organization sharing reports that explicitly and keeps the file private; the
other three sharing levels are unaffected.

## Security notes — protect the refresh token

Flameshot stores the OAuth **refresh token** in its plaintext configuration
file, consistent with how it stores other settings (and with tools such as
rclone). The blast radius is limited by the granted scopes: a leaked token
reaches only files Flameshot itself created — but those are exactly your
captured screenshots — and, since recipient suggestion was added, read-only
lookups of your organization's directory and the user's own group memberships.
None of the scopes can modify anything beyond Flameshot's own files. Treat the
refresh token as the value to protect, not the client ID/secret.

Recommendations:

- **Exclude the Flameshot config file from dotfile sync and backups.** On
  Linux it lives under `~/.config/flameshot/`, on Windows under
  `%APPDATA%\flameshot\`.
- Flameshot re-asserts owner-only permissions on the config file after each
  save where the platform supports it.
- **If you suspect exposure**, use **Disconnect** in Flameshot (which revokes
  the token) and, if needed, revoke the app from your
  [Google account security page](https://myaccount.google.com/permissions).
- Capability-URL leakage (via clipboard managers or the history cache) only
  matters for the "anyone on the internet" level; the other levels still
  require the granted permission to open the file.

## Packaging notes

- `ENABLE_GDRIVE` mirrors the existing `ENABLE_IMGUR` build option and defaults
  **OFF**. Packagers who enable Imgur uploads should decide, per package,
  whether to also enable Google Drive uploads (`-DENABLE_GDRIVE=ON`); the two
  are independent and can be toggled separately.
- HTTPS to Google's APIs requires TLS. On Windows this means the optional
  **OpenSSL** dependency must be shipped/required when `ENABLE_GDRIVE=ON`;
  Flameshot preflights TLS support and shows a clear remediation error rather
  than failing opaquely mid-authorization.
