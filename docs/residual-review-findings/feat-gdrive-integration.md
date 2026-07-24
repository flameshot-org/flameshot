# Residual review findings — feat/gdrive-integration (Google Drive upload)

Recorded from the `ce-work` code-review pass (correctness, security, reliability,
adversarial reviewers) over the Google Drive upload feature. The high- and
medium-severity findings were fixed in commit `594b9d1f` and the pre-existing
UB in a follow-up commit. The items below were **accepted as residuals** (not
fixed) with rationale, so they are recorded rather than dropped. This branch was
committed locally only (no PR opened), per the user's choice.

## Accepted residuals

### R-A. Concurrent first-run uploads can create duplicate "Flameshot screenshots" folders — P2
- **Where:** `src/tools/imgupload/storages/gdrive/gdriveuploader.cpp` (`ensureFolder`/`findOrCreateFolder`/`createFolder`).
- **Finding:** Two captures fired before the folder ID is cached (fresh install, or right after the 404 re-discovery path clears it) can each list-then-create, producing two folders; the last write to `gdriveFolderId` wins.
- **Why accepted:** This is the exact race the plan's **KTD5** acknowledges and accepts as a one-time, first-use event ("reduces the non-atomic find-or-create race to a one-time event"). It is a session-settled design decision, not a new defect. Adding a cross-widget folder single-flight (mirroring the OAuth waiter pattern) is the future hardening if it proves to matter in practice.

### R-B. Loopback listener parses a single `readyRead` chunk — P3
- **Where:** `src/tools/imgupload/storages/gdrive/gdriveoauth.cpp` (`onLoopbackConnection`).
- **Finding:** The HTTP request line is parsed from one `readyRead` read. If the browser's GET were split across TCP segments, the redirect could be missed and recovery would fall to the ~3-minute consent timeout.
- **Why accepted:** On the loopback interface the tiny request line arrives in a single segment in practice (loopback MTU is large); the pathological split still recovers (via timeout) rather than corrupting state. Low probability, low impact. Future hardening: accumulate per-socket bytes until a full request line is seen.

### R-C. Stale folder/account cache heals only on 404, not 403 — P3 (residual risk)
- **Where:** `gdriveuploader.cpp` resumable-upload error handling.
- **Finding:** A stale cached `gdriveFolderId` triggers re-discovery on HTTP 404; a 403-style response would not.
- **Why accepted:** 404 is the expected signal for a deleted/inaccessible folder; 403 (policy) is a different condition where re-discovery would not help. Low impact.

### R-D. GDrive-only build routes legacy Imgur history entries to the Drive backend — P3 (residual risk)
- **Where:** `imguploadermanager.cpp` `init()` fallback + `gdriveuploader.cpp` `deleteImage`.
- **Finding:** In an `ENABLE_GDRIVE`-only build (no Imgur), deleting a pre-existing Imgur history entry hex-decodes the Imgur deletehash as a Drive file ID → garbage/empty ID → the Drive delete no-ops (404 treated as already-gone) and the row is removed without deleting anything real.
- **Why accepted:** Requires a cross-build-flavor scenario (Imgur history carried into a Drive-only build) that is rare; it does not crash and the user's intent (remove the row) is honored. The Imgur file cannot be deleted from a Drive-only build regardless.

### R-E. Server-side revocation failure on Disconnect is silent — P3 (residual risk)
- **Where:** `gdriveoauth.cpp` `disconnectAccount`.
- **Finding:** The best-effort POST to Google's `/revoke` endpoint has no retry or user-facing signal if it fails; local credentials are still cleared (correct), but the user isn't told server-side revocation itself may not have happened.
- **Why accepted:** Local disconnect is the primary guarantee; the docs already direct users to the Google account security page for definitive revocation after suspected exposure.

### R-F. Drive API failures collapse into one opaque message — P2 (deferred, user's choice)
- **Where:** `src/tools/imgupload/storages/gdrive/gdriveuploader.cpp` (`findOrCreateFolder`, and the same pattern in `createFolder`/`startResumableUpload`/`putBytes`/`fetchLink`); `gdriveoauth.cpp` `fetchAccountInfo` swallows its failure entirely as best-effort.
- **Finding:** Every reply error becomes a fixed string — `"Could not access Google Drive."` — with the HTTP status and Google's `error.message` body discarded. A first-run failure (for example the Drive API not enabled on the Cloud project, a scope not granted, or a proxy blocking `www.googleapis.com`) is therefore indistinguishable from any other, and the user has no signal to act on. This came up in a real diagnosis: the message was reached *after* a fully successful OAuth consent and token exchange, but nothing in the UI or logs revealed which Drive call failed or why.
- **Why deferred:** Surfacing the status/error body was offered alongside the loopback-response fix and the user scoped this session to the socket bug only. Left recorded rather than dropped. Future fix: include the HTTP status code and Google's `error.message` in the failure text (never the access token), and log `fetchAccountInfo`'s failure instead of silently continuing.

## Notes
- The feature has **no automated test coverage** — the repo has no C++ test framework and the plan introduces none (Verification Contract: build matrix + manual acceptance). This is a plan-level accepted condition, not a review residual.
- The plan's acceptance walkthrough (AE1–AE9) and failure-path drill require a real Google Workspace account, an org-registered Desktop OAuth client, and a browser; they must be executed by a human before merge.
