---
title: "QSettings deferred write defeats chmod-based config permission hardening"
module: confighandler
date: 2026-07-24
problem_type: security_issue
component: authentication
severity: high
symptoms:
  - "A secret written via ConfigHandler setters and then chmod'd to 0600 ends up world-readable (0644) on disk"
  - "The permission hardening looks correct in code but the file mode is wrong after the process settles"
root_cause: async_timing
resolution_type: code_fix
tags:
  - qsettings
  - file-permissions
  - config
  - oauth
  - refresh-token
  - umask
related_components:
  - tooling
---

# QSettings deferred write defeats chmod-based config permission hardening

## Problem

Hardening a secret in Flameshot's config by calling `QFile::setPermissions(path, ReadOwner | WriteOwner)` right after writing it through `ConfigHandler` does **not** guarantee an owner-only file. `ConfigHandler`'s setters do not flush, so the chmod runs before the real disk write and the later write resets the mode.

## Symptoms

- A value written through a `ConfigHandler` setter and then "protected" with `QFile::setPermissions(..., ReadOwner | WriteOwner)` is still world-readable (typically `0644`) once the process settles.
- The hardening code reads correctly and the chmod call returns success, yet the on-disk mode is wrong — so the bug is invisible unless you inspect the file mode after a real (non-mocked) write.

## What Didn't Work

Calling the chmod immediately after the setter, in the same scope, while the `ConfigHandler` (and its `QSettings`) is still alive:

```cpp
ConfigHandler config;
config.setGdriveRefreshToken(refresh);          // does NOT write to disk yet
reassertConfigPermissions(config.configFilePath()); // chmod 0600 — too early
```

`ConfigHandler::setValue` only calls `m_settings.setValue(key, val)` with no `sync()` (`src/utils/confighandler.cpp:510`). `QSettings` defers the actual write to its auto-save timer or to destruction. So at the moment of the chmod either (a) the file has not been rewritten yet, or (b) it is rewritten *afterwards*. Because `QSettings` writes ini files with `QSaveFile` (write a temp file, then atomically rename it over the original), the new file's mode comes from the process **umask** (usually `0644`), not from the previous file — the later rewrite silently discards the `0600` the chmod just set.

## Solution

Force the write to complete, then chmod. A `flush()` was added to `ConfigHandler` and every Drive persist path flushes before re-asserting permissions.

```cpp
// src/utils/confighandler.cpp
void ConfigHandler::flush()
{
    m_settings.sync();
}
```

```cpp
// src/tools/imgupload/storages/gdrive/gdriveoauth.cpp
ConfigHandler config;
config.setGdriveRefreshToken(refresh);
config.flush();                                  // write now (0644 via umask)
reassertConfigPermissions(config.configFilePath()); // then chmod 0600 — holds
```

`reassertConfigPermissions` itself just calls `QFile::setPermissions(path, ReadOwner | WriteOwner)` (no `QFile::exists()` pre-check — `setPermissions` no-ops harmlessly on a missing path, and the pre-check is racy).

Fixed on the `feat/gdrive-integration` branch (commit `594b9d1f`, unmerged as of this writing).

## Why This Works

`sync()` performs the deferred `QSaveFile` write synchronously, so by the time the chmod runs the file exists with its final contents (at umask mode). The chmod then tightens it to `0600`, and because no further `ConfigHandler` write happens in that scope, nothing rewrites the file afterward — the destructor's `sync()` is a no-op when there are no pending changes, so it does not clobber the mode. Order matters: **write, then chmod**, never chmod-then-write.

## Prevention

- **Any "write a secret then tighten its file mode" sequence on a `QSettings`-backed store must flush between the two steps.** `setValue` is not a durable write.
- **Prefer removing the race at the source for process-global protection:** call `::umask(0077)` once at startup so every `QSaveFile` temp file is created `0600` regardless of write timing. Flameshot did **not** take this route because the umask is process-wide and would also restrict user-saved screenshot files, which is undesirable; the targeted flush-then-chmod keeps the scope to the config file. Choose per blast radius: umask for "everything this process writes should be private", flush-then-chmod for "just this file".
- **Test the on-disk mode after a real sync, not the chmod call.** A test that only asserts `reassertConfigPermissions()` no-ops on a missing file would pass while the real bug (mode reset by the deferred rewrite) ships. Assert `QFile(path).permissions()` after `setGdriveRefreshToken()` + a real `sync()`.
- **Plaintext secrets in this config are an accepted tradeoff (plan KD5), so file permissions are the only local barrier** — getting them wrong removes the one protection the design relies on. Treat the refresh token, not the client id/secret, as the value to protect.
