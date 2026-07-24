---
title: Capture-Completion Lifecycle - Plan
type: refactor
date: 2026-07-24
topic: capture-completion-lifecycle
artifact_contract: ce-unified-plan/v1
artifact_readiness: requirements-only
product_contract_source: ce-brainstorm
execution: code
---

# Capture-Completion Lifecycle - Plan

## Goal Capsule

- **Objective:** Move the GUI export/upload flow off `CaptureWidget`'s destructor
  stack so it no longer runs a modal event loop and object teardown from inside
  `~CaptureWidget` while that destructor is being executed by `sendPostedEvents`.
- **Product authority:** `Flameshot` owns the post-capture lifecycle (widget
  teardown + export dispatch). Not in active scope: the CLI export paths
  (`screen()`/`full()`), the upload backends, and making the confirmation dialog
  non-modal.
- **Open blockers:** None. All product decisions are settled; remaining items are
  deferred to planning.

## Product Contract

### Summary

`CaptureWidget` stops invoking `exportCapture` from its destructor. It emits a
completion (or failure) signal carrying the finished capture; `Flameshot` — which
already creates and owns the capture window — receives that signal, runs the
export/upload flow on a clean event-loop turn, then tears the widget down. This
removes the re-entrancy that segfaults on Google Drive upload confirmation, with
no change to observable behavior.

### Problem Frame

Confirming a Google Drive upload crashes the app. The confirmed root cause is a
re-entrancy hazard, not a logic error in the upload code.

Selecting the upload tool sets the capture as done and closes the capture window.
`CaptureWidget` has `WA_DeleteOnClose`, so Qt posts a deferred-delete event; the
event loop's `sendPostedEvents` later runs `~CaptureWidget`. The destructor then
calls `Flameshot::exportCapture`, which — for the upload task — opens a **modal**
confirmation dialog (`ImgUploadDialog::exec()`, a nested event loop) and creates
a self-deleting uploader widget with `deleteLater`.

Running a nested modal loop and posting deferred-delete events from *inside a
destructor that is itself being run by `sendPostedEvents`* corrupts the posted-event
bookkeeping. The captured backtrace crashes in
`QCoreApplication::postEvent → lockThreadPostEventList` with an atomic decrement on
a poisoned pointer (`0xffff000000000000`), reached from
`~CaptureWidget → exportCapture` under `sendPostedEvents`. The immediate trigger is
a use-after-free of the confirmation dialog (the core dump shows the dialog's
visibility read back empty even with Google Drive active), but the underlying fault
is that export runs on the destructor's stack at all.

### Key Decisions

- **`Flameshot` owns the post-capture lifecycle.** It connects to the widget's
  completion/failure signals at window-creation time and drives export + teardown.
  (session-settled: user-directed — chosen over a dedicated controller class:
  simpler, and `Flameshot` is already the de-facto owner of the window and
  `exportCapture`.) Governs R4, R5, R6, R7.
- **Export runs on a clean event-loop turn.** The completion signal hands off so
  `exportCapture` executes after `~CaptureWidget` returns and `sendPostedEvents`
  unwinds, never nested inside widget teardown. (session-settled: user-approved —
  preserves the "close then export" ordering while severing the re-entrancy.)
  Governs R5, R9.
- **Teardown ownership moves to `Flameshot`.** `WA_DeleteOnClose` comes off
  `CaptureWidget`; the widget is destroyed explicitly after completion handling.
  Governs R1, R6.
- **`exportCapture` stays the shared task executor.** The CLI paths keep calling it
  directly; only the GUI invocation site changes. Governs R12.
- **The confirmation dialog gets a deterministic lifetime.** `ImgUploadDialog`'s
  own `WA_DeleteOnClose` is removed and its lifetime scoped, so the post-`exec()`
  reads are never a use-after-free. Governs R13.

### Requirements

**Capture-completion signaling**

- R1. `CaptureWidget` must not call `exportCapture` or any export logic from its
  destructor; the destructor carries no capture-completion side effects.
- R2. On a completed capture, `CaptureWidget` emits a completion signal carrying
  the finished pixmap, the computed target geometry, and the `CaptureRequest` by
  value. On a non-completed capture it emits failure.
- R3. The final pixmap and geometry currently derived in the destructor (region
  scaling by device pixel ratio, `widgetOffset`) are derived at the completion
  point and carried in the signal, not recomputed after teardown.

**`Flameshot` ownership**

- R4. `Flameshot` connects to the widget's completion/failure signals when it
  creates the capture window in `gui()`.
- R5. On completion, `Flameshot` runs `exportCapture` on a clean event-loop turn —
  outside the widget's close/teardown call stack.
- R6. `Flameshot` owns widget teardown: the capture window is destroyed after
  completion handling via `deleteLater`, not via `WA_DeleteOnClose` or an
  export-in-destructor.
- R7. On failure or cancellation, `Flameshot` tears the widget down and emits the
  existing `captureFailed()` signal.

**Behavior preservation**

- R8. The modal confirmation dialog, the upload-without-confirmation path, and the
  SAVE / COPY / PIN / PRINT tasks behave exactly as today from the user's
  perspective. Covered by AE1–AE3.
- R9. The capture overlay closes before the export/upload UI appears (current
  ordering preserved). Covers KD "clean turn".
- R10. The GNOME/Wayland clipboard workaround — the window stays alive until
  clipboard data is read — is preserved. Covered by AE4.
- R11. `gui()`'s return of the capture widget (trayicon uses it as the
  update-notification parent) and the single-active-capture-window guard are
  unchanged.
- R12. The CLI paths (`screen()`, `full()`) and the upload backends are unchanged;
  `exportCapture` remains shared.

**Confirmation-dialog lifetime**

- R13. `ImgUploadDialog` is never accessed (`selectedVisibility()`, `recipients()`,
  `deleteLater()`) after it may have auto-destroyed. Its `WA_DeleteOnClose` is
  removed and its lifetime scoped so the post-`exec()` reads are safe. Covered by
  AE5.

### Key Flows

- F1. **GUI capture → export (happy path).** **Trigger:** user accepts a tool that
  requests an export task (e.g. upload). Sequence: capture marked done → close
  requested → overlay closes → widget emits completion with the finished capture →
  `Flameshot` handles it on a clean turn → `exportCapture` runs (confirmation
  dialog if enabled, then the uploader/save/copy/pin work) → `Flameshot` tears the
  widget down. **Covers R1, R2, R5, R6, R9.**
- F2. **Failure / cancellation.** **Trigger:** capture ends without completion, or
  the user rejects the confirmation dialog. `Flameshot` tears the widget down and
  emits `captureFailed()`; no export proceeds. **Covers R7.**

### Acceptance Examples

- AE1. **Covers R5, R8, R9.** Confirming a Google Drive upload does not crash; the
  capture overlay is gone before the confirmation dialog appears; the upload
  proceeds. (This is the reported regression.)
- AE2. **Covers R8.** With "upload without confirmation" enabled, no dialog appears
  and the upload proceeds directly.
- AE3. **Covers R8, R13.** Rejecting the confirmation dialog performs no upload,
  does not crash, and the capture window is torn down.
- AE4. **Covers R10.** On GNOME/Wayland with a COPY task, the capture window stays
  alive until the clipboard data is fetched, then the capture completes normally.
- AE5. **Covers R13.** Confirming a Drive upload with a non-default visibility
  yields the selected visibility (not an empty value), because the dialog is alive
  when its selection is read.

### Scope Boundaries

- Making the confirmation dialog non-modal — deferred; this change makes it
  possible later but keeps the dialog modal (now on a safe stack).
- Refactoring the CLI export paths (`screen()`/`full()`) — out; they already call
  `exportCapture` directly and synchronously.
- Changes to the upload backends or Google Drive logic — out.
- Broader `CaptureWidget` decomposition — out.

### Sources / Research

- `src/widgets/capture/capturewidget.cpp:317-331` — destructor calls
  `exportCapture` on completion; `:625-657` — `closeEvent` GNOME/Wayland clipboard
  workaround; `:1439-1456` — `REQ_CLOSE_GUI` / `REQ_CAPTURE_DONE_OK` handling;
  `:101` — `WA_DeleteOnClose`.
- `src/core/flameshot.cpp:123-182` — `gui()` creates/owns `m_captureWindow`
  (`QPointer`), single-window guard, cross-platform show, returns the widget;
  `:450-536` — `exportCapture` (SAVE/COPY/PIN/PRINT/UPLOAD); `:502-508` — the
  post-`exec()` dialog access that crashed; `:232`,`:248` — CLI call sites.
- `src/widgets/imguploaddialog.cpp:24` — dialog `WA_DeleteOnClose`.
- `src/widgets/trayicon.cpp:284-286` — consumes `gui()`'s returned widget as the
  update-notification parent.
- Crash evidence: backtrace bottoms out in `sendPostedEvents → QObject::event →
  ~CaptureWidget (capturewidget.cpp:327) → exportCapture (flameshot.cpp:508) →
  postEvent → lockThreadPostEventList`; poisoned-pointer atomic decrement.

### Outstanding Questions

**Deferred to Planning**

- Exact emission point inside `CaptureWidget` (a dedicated finish method vs.
  `closeEvent`) and the clean-turn mechanism (queued-connection signal vs.
  single-shot timer).
- How `Flameshot` tracks the per-capture widget for teardown alongside the existing
  `m_captureWindow` `QPointer`, and where the current destructor `captureFailed()`
  emission moves to.
- Whether `exportCapture`'s GUI-only concerns (it is shared with CLI) need any
  guard once invoked from the completion handler rather than the destructor.
