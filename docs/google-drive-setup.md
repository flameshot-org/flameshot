# Google Drive upload — setup guide

Flameshot can upload captures to Google Drive as an alternative to Imgur,
for organizations that want screenshots to stay inside their Google Workspace
domain. This is an opt-in build feature (`-DENABLE_GDRIVE=ON`) that uses an
OAuth client **you register in your own Google Cloud project** — nothing is
shipped in the Flameshot binary.

## What you get

- Uploads go into a single folder named **"Flameshot screenshots"** in the
  signed-in user's own My Drive.
- Flameshot is confined by the least-privilege `drive.file` scope: it can only
  see and touch files it created. It cannot list or open anything else in your
  Drive, and has no access to shared/Team Drives.
- Per-upload sharing visibility, with a configurable default:
  - Anyone in your organization with the link (the shipped default)
  - Private (only you)
  - Specific people by email (individuals or groups)
  - Anyone on the internet with the link

## For the administrator: register the OAuth client

You need a Google Cloud project and an OAuth client of type **Desktop app**.
This is a one-time setup, external to Flameshot.

1. Open the [Google Cloud console](https://console.cloud.google.com/) and
   create a project (or reuse one) inside your Workspace organization.
2. Enable the **Google Drive API** for that project
   (*APIs & Services → Library → Google Drive API → Enable*).
3. Configure the **OAuth consent screen**:
   - Choose **Internal** user type. An Internal Workspace app skips Google's
     verification entirely and is not subject to the 7-day testing-mode
     refresh-token expiry.
   - Add the scope `.../auth/drive.file`. This scope is classified
     **non-sensitive**, so no restricted-scope verification is required.
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

## Security notes — protect the refresh token

Flameshot stores the OAuth **refresh token** in its plaintext configuration
file, consistent with how it stores other settings (and with tools such as
rclone). The blast radius is limited by the `drive.file` scope — a leaked
token only reaches files Flameshot itself created — but those are exactly your
captured screenshots, so treat the refresh token as the value to protect, not
the client ID/secret.

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
