# Concepts

Shared domain vocabulary for this project — entities, named processes, and status concepts with project-specific meaning. Seeded with core domain vocabulary, then accretes as ce-compound and ce-compound-refresh process learnings; direct edits are fine. Glossary only, not a spec or catch-all.

## Uploads

### Uploader
The backend-neutral subsystem that sends a captured screenshot to a remote destination and exposes the surrounding machinery: the upload tool, its keyboard shortcut, the upload dialogs, and upload history. It is present in a build only when at least one upload backend is enabled, and its machinery is shared across backends rather than tied to any one of them.

### Upload backend
A concrete remote destination the Uploader can target. Flameshot has two — Imgur (an image host) and Google Drive (cloud file storage). Each backend is opt-in at build time and off by default, so a default build ships with no upload capability at all; enabling any backend also brings in the neutral Uploader machinery. Because the two backends are independent switches, a build may include one, both, or neither.
