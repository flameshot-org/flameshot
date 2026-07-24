---
title: "Google Drive visibility controls silently missing from upload dialog on Drive-only builds"
date: 2026-07-24
category: logic-errors
module: "Google Drive upload — sharing UI"
problem_type: logic_error
component: tooling
symptoms:
  - "Upload confirmation dialog shows no visibility combo box or recipients field, even though the screenshot is actively uploading to Google Drive"
  - "Post-upload status label reads \"Shared: anyone in your organization with the link\" despite the user never being offered a visibility choice"
  - "Behavior reproduces reliably on builds compiled with ENABLE_GDRIVE=ON and ENABLE_IMGUR=OFF"
root_cause: logic_error
resolution_type: code_fix
severity: medium
related_components:
  - imguploadermanager
  - confighandler
  - gdriveuploader
  - generalconf
tags:
  - gdrive
  - imguploaddialog
  - imguploadermanager
  - backend-resolution
  - qt6
  - upload-visibility
  - config-drift
---

# Google Drive visibility controls silently missing from upload dialog on Drive-only builds

## Problem

`ImgUploadDialog` decided whether to show the Google Drive sharing/visibility
controls by re-checking the raw `uploadStorage` config key itself, instead of
asking the same resolver (`ImgUploaderManager`) that the actual upload path
uses — so on a Drive-only build, uploads went to Google Drive while the
dialog's own check still believed Drive wasn't active, and it silently
skipped the sharing UI.

## Symptoms

- **Round 1**: no confirmation dialog appeared at all — the app went
  straight from clicking upload to a spinner ("Uploading image...") to the
  completed upload widget.
- **Round 2** (after removing `uploadWithoutConfirmation=true`): the
  confirmation dialog appeared ("Подтверждение отправки" / "Confirm to
  send", with Yes/No and an "Upload without confirmation" checkbox), but it
  still showed no visibility `QComboBox` and no recipients field — despite
  the user's config genuinely having `gdriveAccountDomain`,
  `gdriveClientId`, `gdriveClientSecret`, `gdriveFolderId`,
  `gdriveGrantedScopes`, and `gdriveRefreshToken` all populated.
- A separate, transient "Shared: anyone in your organization with the link"
  line appeared briefly on the post-upload result widget and faded; the
  user reported it looked "static" and unresponsive to clicks.

## What Didn't Work

The first hypothesis — that `ConfigHandler().uploadWithoutConfirmation()`
was `true` in the user's config — explained Round 1 completely: with that
flag set, `src/core/flameshot.cpp:569` (`if (!ConfigHandler().uploadWithoutConfirmation())`)
skips constructing `ImgUploadDialog` entirely, so no dialog and no sharing
choice ever appear, confirmed once the user pasted their real, redacted
`flameshot.ini`.

That hypothesis stopped explaining anything once the user removed the flag
for Round 2. The dialog now appeared as expected, which ruled out the
confirmation-skip path as the (sole) cause — but the visibility combo box
and recipients field were still missing even though every other piece of
Drive config was present and non-empty. This was the pivot point: the
missing-dialog symptom and the missing-controls-within-the-dialog symptom
turned out to be two different bugs (one config-driven, one a genuine
backend-detection bug), and continuing to chase the confirmation-flag
theory in Round 2 would have been a dead end.

A second thing that needed ruling out (not a real dead end, but a
disambiguation) was whether the fading "Shared: anyone in your organization
with the link" text was itself a broken/non-interactive sharing control.
Tracing `GDriveUploader::finalizeSuccess()`
(`src/tools/imgupload/storages/gdrive/gdriveuploader.cpp:467-495`) showed
the message is built via `tr("Shared: %1").arg(visibilityDescription())`
and delivered through either `notification()->showMessage(message)` or
`setInfoLabelText(message)`
(`src/tools/imgupload/storages/imguploaderbase.cpp`), gated on
`ConfigHandler().copyURLAfterUpload()` — both are read-only, non-interactive
status displays by design. This was confirmed to be working as intended
(informing the user what visibility was actually applied), not a bug.

## Solution

`src/widgets/imguploaddialog.cpp:19` initialized `m_driveActive` from the
raw config value directly:

```cpp
// Before:
ImgUploadDialog::ImgUploadDialog(QDialog* parent)
  : QDialog(parent)
  , m_driveActive(ConfigHandler().uploadStorage() == QStringLiteral("gdrive"))
  , m_visibility(nullptr)
  ...

// After:
#include "tools/imgupload/imguploadermanager.h"
...
ImgUploadDialog::ImgUploadDialog(QDialog* parent)
  : QDialog(parent)
  // Ask the same resolver the uploader itself uses (ImgUploaderManager),
  // rather than re-deriving backend selection from the raw config value:
  // on a build with only one backend compiled in, that backend is always
  // the effective one regardless of what "uploadStorage" happens to hold.
  , m_driveActive(ImgUploaderManager().uploaderPlugin() ==
                  QStringLiteral("gdrive"))
  , m_visibility(nullptr)
  ...
```

`ImgUploaderManager::uploaderPlugin()` (declared in
`src/tools/imgupload/imguploadermanager.h:31`) already existed as a public
getter returning the plugin string resolved by `init()` in the
constructor — it just wasn't being used here. The fix was verified to
compile clean via `cmake --build obj-x86_64-linux-gnu --target flameshot`.
The diff was committed locally on the `feat/gdrive-integration` branch as
`b8186ea1` ("fix(gdrive): resolve active backend via ImgUploaderManager,
not raw config"), scoped to exactly this file: 7 insertions, 1 deletion —
the new include, the explanatory comment, and the changed initializer. Not
yet pushed or opened as a PR as of this writing, so treat the SHA as
local-only and subject to change on rebase/squash.

## Why This Works

Two code paths were independently answering the same conceptual question —
"is Google Drive the active upload backend?" — using different logic, and
only one of them accounted for the single-backend-compiled-in fallback:

1. `src/config/generalconf.cpp:524-557` (`GeneralConf::initUploadService`)
   is the *only* place in the codebase that calls
   `ConfigHandler().setUploadStorage(...)`, and it does so inside the
   "Upload service" `QComboBox`'s `currentIndexChanged` handler. Connecting
   a Google account (entering OAuth client id/secret, completing the OAuth
   flow, populating `gdriveClientId`, `gdriveClientSecret`,
   `gdriveRefreshToken`, `gdriveAccountEmail`, `gdriveAccountDomain`, etc.)
   never touches `uploadStorage`. A user can fully configure Drive without
   ever selecting "Google Drive" from that dropdown, leaving
   `uploadStorage` at its config default of `"imgur"`.

2. `src/tools/imgupload/imguploadermanager.cpp:27-52`
   (`ImgUploaderManager::init()`) is the code that actually decides which
   backend performs the upload: if the plugin var is `"gdrive"`, use
   gdrive; else if `ENABLE_IMGUR` is compiled in, force `"imgur"`; else
   (only `ENABLE_GDRIVE` compiled in) force `"gdrive"` regardless of the
   configured value. This build's `obj-x86_64-linux-gnu/CMakeCache.txt`
   confirms `ENABLE_GDRIVE:BOOL=ON` and `ENABLE_IMGUR:BOOL=OFF` — a
   Drive-only build — so `ImgUploaderManager` always resolved to `gdrive`
   regardless of `uploadStorage`.

`ImgUploadDialog`'s old check (`ConfigHandler().uploadStorage() ==
"gdrive"`) had no knowledge of that single-backend fallback branch, so it
evaluated `false` whenever `uploadStorage` hadn't been explicitly set —
exactly the state left behind by the OAuth-connect flow in (1). Both code
paths compiled and ran without error; they simply disagreed silently,
which is why the bug only surfaced as a missing UI element rather than a
crash or visible error.

**(session history) This exact defect class was already known in this
codebase before the dialog was even written.** Prior-session history for
this feature's brainstorm → plan → implementation arc shows:

- During planning, a feasibility review already caught that a GDRIVE-only
  build's default `uploadStorage="imgur"` config value would route to a
  backend that isn't even compiled in — the earliest recognition that the
  raw `uploadStorage` value does not reliably describe which backend is
  actually active.
- During the same planning pass, an architecture review separately caught
  that `ImgUploaderManager`'s string-keyed overload could self-clobber via
  `init()` and misroute history deletes — a different bug, but in the same
  file, from the same root cause family (backend-resolution state handled
  inconsistently across call paths).
- `ImgUploaderManager` was explicitly designed, from the brainstorm onward,
  to be the single source of truth for backend resolution. But
  implementation split the work into separate units executed in the same
  session — the manager rewrite (backend resolution) landed first, and the
  dialog (sharing UI, including `m_driveActive`) was written afterward as a
  separate unit, with no visible step cross-checking "how does the dialog
  know Drive is active" against "how the manager decides which backend is
  active." A subsequent code-review pass in that same implementation
  session caught several related defects in the sharing UI (e.g. a default
  visibility option with no default recipients) but did not catch this
  particular divergence.

In other words, this was the second time this exact divergence — a raw
config value disagreeing with actually-resolved backend state — surfaced
in this one feature's development (the first, caught during planning's
feasibility review), plus a third, related-but-distinct bug from the same
root-cause family (the manager's self-clobbering overload, caught during
architecture review). The first two were caught before anything shipped;
this one shipped and had to be caught by a user running the built app.

## Prevention

- Don't let backend/state resolution logic exist in more than one place.
  When a decision has a non-trivial rule — especially a fallback branch
  like "if only one backend is compiled in, that backend wins regardless of
  the stored preference" — any other code asking the same question must
  call the shared resolver (`ImgUploaderManager::uploaderPlugin()`) rather
  than recomputing the condition from raw config.
- Concretely, `ImgUploadDialog` and `ImgUploaderManager` are the two known
  call sites that answer "is Drive active?" today. If a third place in the
  codebase ever needs to ask the same question, grep for
  `uploadStorage() ==` and `uploaderPlugin() ==` first, and route the new
  check through `ImgUploaderManager` instead of adding a third
  independent derivation.
- When a build can compile in a subset of optional backends/features
  (`ENABLE_GDRIVE`, `ENABLE_IMGUR`, and similar `#ifdef`-gated options),
  treat "effective backend" as a single source of truth exposed via one
  getter, and audit for any UI/gating code that reads the raw preference
  key directly instead of that getter.
- (session history) When a plan or architecture review already flags a
  defect class once (here: raw config vs. resolved state divergence,
  caught twice during planning for this same feature), treat that as a
  standing risk for the rest of the feature's implementation, not a
  one-off fix — specifically, re-check any later-written unit that reads
  the same config key directly. A short cross-unit review step ("does this
  new code re-derive a condition another component already owns resolving?")
  would likely have caught this before it shipped.

## Related Issues

- [`docs/solutions/developer-experience/building-the-deb-with-gdrive-support.md`](../developer-experience/building-the-deb-with-gdrive-support.md) —
  a complementary failure mode for the same subsystem: that doc covers
  Google Drive support not being *compiled in at all* (`ENABLE_GDRIVE`
  defaults OFF); this doc covers Google Drive being compiled in and
  actively used, but its sharing UI hiding itself anyway due to a
  backend-resolution mismatch. A reader debugging "why is my Drive UI
  missing" could plausibly land on either doc first.
- No other existing `docs/solutions/` entries were found to overlap with
  this problem (checked against
  `docs/solutions/developer-experience/building-the-deb-package.md`
  and `docs/solutions/security-issues/qsettings-deferred-write-defeats-permission-chmod.md`
  — both address unrelated mechanisms).
