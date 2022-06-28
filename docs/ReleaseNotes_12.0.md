# Version 12.0.rc1 Beta
This is the beta for the version 12.0 release. We will be in beta for about a week unless major issues are discovered.

## New Features
- Created basic layer movement functionality (up, down) by @affirVega in https://github.com/flameshot-org/flameshot/pull/2108
<p align=center><img src="https://github.com/flameshot-org/flameshot/blob/master/docs/images/layer.gif" width=50%> </p>

- Added a new widget to allow the colorwheel to be more easily customized by https://github.com/flameshot-org/flameshot/pull/2202
<p align=center><img src="https://github.com/flameshot-org/flameshot/blob/master/docs/images/colorwheel.png" width=50%> </p>


- Added magnifier for more precise selections by @SilasDo in https://github.com/flameshot-org/flameshot/pull/2219
  - The new magnifier can be enabled in ```Configuration > General > Show Magnifier``` 
  - There is an option to make the magnifier a square or circle
<p align=center><img src="https://github.com/flameshot-org/flameshot/blob/master/docs/images/magnifer.gif" width=50%> </p>

- Incremental markers can now have a point if you drag when placing them. @vozdeckyl in https://github.com/flameshot-org/flameshot/pull/2638
<p align=center><img src="https://github.com/flameshot-org/flameshot/blob/master/docs/images/number_pointer.png" width=50%> </p>

- Added the ability to cache the last region by @borgmanJeremy in https://github.com/flameshot-org/flameshot/pull/2615
  - The launcher tool will automatically populate the coordinates for the last selection region
  - If ```Configuration > General > Use last region``` is selected, Flameshot will always initialize with the last successfully captured region
<p align=center><img src="https://github.com/flameshot-org/flameshot/blob/master/docs/images/region_launcher.png" width=50%> </p>

- Pinned screenshots can now be copied to the clipboard or saved to a file if a user right clicks on the pinned image by @zhangfuwen in https://github.com/flameshot-org/flameshot/pull/2519

- Users can now specify their own Imgur API Key from ```Configuration > General > Imgur API Key```. This is encouraged because as Flameshot has gotten more popular we have started exceeding the upload limit of the default API key by@borgmanJeremy in https://github.com/flameshot-org/flameshot/pull/2503

- Added 'Save to disk' button when uploading to imgur by @AndreaMarangoni in https://github.com/flameshot-org/flameshot/pull/2237

- Pinned screenshots can now be zoomed with a pinch gesture by @AndreaMarangoni in https://github.com/flameshot-org/flameshot/pull/2447

- The SVG's have been optimized by @RiedleroD in https://github.com/flameshot-org/flameshot/pull/2318

- Make KDE use Freedesktop portal by @greenfoo in https://github.com/flameshot-org/flameshot/pull/2495

- Allow final actions when printing geometry when invoke by CLI by @borgmanJeremy in https://github.com/flameshot-org/flameshot/pull/2444

- Many Flameshot widgets have been reworked to use .ui XML files and Qt Designer. This has been done to allow non C++ developers to more easily contribute to the graphical side of Flameshot.
<p align=center><img src="https://github.com/flameshot-org/flameshot/blob/master/docs/images/ui_file.png" width=50%> </p>

- Updated Translations

# Bug Fixes
- Pinned images can now be moved partially offscreen on linux by @zhangfuwen in https://github.com/flameshot-org/flameshot/pull/2520
- Wayland builds now use KF Gui (KDE Framework tools) to fix some issues by @borgmanJeremy in https://github.com/flameshot-org/flameshot/pull/2305
- Fix Flameshot crashes with GB locale by @AndreaMarangoni in https://github.com/flameshot-org/flameshot/pull/2304
- Add alternative shortcuts file for KDE Flatpak installs by @Proton-459 in https://github.com/flameshot-org/flameshot/pull/2357
- fixed freeze with copy URL to clipboard by @borgmanJeremy in https://github.com/flameshot-org/flameshot/pull/2348
- Fixed crash selecting texttool by @AndreaMarangoni in https://github.com/flameshot-org/flameshot/pull/2369
- Improve tooltips texts by @mmahmoudian in https://github.com/flameshot-org/flameshot/pull/2377
- better zsh code completion by @mmahmoudian in https://github.com/flameshot-org/flameshot/pull/2382
- Print info messages to stdout instead of stderr by @borgmanJeremy in https://github.com/flameshot-org/flameshot/pull/2639
- Fix CloseOnLastWindow caused by tool change by @veracioux in https://github.com/flameshot-org/flameshot/pull/2645
- fix unexpected close when launch external app by @Alaskra in https://github.com/flameshot-org/flameshot/pull/2617
- Fix sidebar slider not resizing by @borgmanJeremy in https://github.com/flameshot-org/flameshot/pull/2530
- fixed segfault when screen number exceeds screen count by @borgmanJeremy in https://github.com/flameshot-org/flameshot/pull/2534
- Remove extra timer shots when moving selection with keyboard by @veracioux in https://github.com/flameshot-org/flameshot/pull/2545
- Fix pinwidget save by @Alaskra in https://github.com/flameshot-org/flameshot/pull/2549
- Config error fix by @vozdeckyl in https://github.com/flameshot-org/flameshot/pull/2552
- Fix missing icon on snap by @vozdeckyl in https://github.com/flameshot-org/flameshot/pull/2616
- Fix selection offset by @veracioux in https://github.com/flameshot-org/flameshot/pull/2630
- Suggest setting XDG_CURRENT_DESKTOP if DE cannot be detected by @greenfoo in https://github.com/flameshot-org/flameshot/pull/2634
- Fix saveAsFileExtension in example config by @veracioux in https://github.com/flameshot-org/flameshot/pull/2414
- fixed high CPU usage on pin by @borgmanJeremy in https://github.com/flameshot-org/flameshot/pull/2502
- Fix alignment bug and applied many clang format warnings by @borgmanJeremy in https://github.com/flameshot-org/flameshot/pull/2448
- fix the --print-geometry for zsh by @mmahmoudian in https://github.com/flameshot-org/flameshot/pull/2437
- fix bug on macos with save dialog by @borgmanJeremy in https://github.com/flameshot-org/flameshot/pull/2379
- allow numpad numbers to resize and fix text artifacting on large resize by @borgmanJeremy in https://github.com/flameshot-org/flameshot/pull/2386
- Zooming in/out happens at different speed by @AndreaMarangoni in https://github.com/flameshot-org/flameshot/pull/2378
- fix: arrow tool glitches by @UnkwUsr in https://github.com/flameshot-org/flameshot/pull/2395
- Fix double click by @borgmanJeremy in https://github.com/flameshot-org/flameshot/pull/2432
- Improve Colorpicker by @deo002 in https://github.com/flameshot-org/flameshot/pull/2403

## New Contributors
* @AndreaMarangoni made their first contribution in https://github.com/flameshot-org/flameshot/pull/2304
* @samrocketman made their first contribution in https://github.com/flameshot-org/flameshot/pull/2311
* @affirVega made their first contribution in https://github.com/flameshot-org/flameshot/pull/2108
* @Proton-459 made their first contribution in https://github.com/flameshot-org/flameshot/pull/2357
* @SilasDo made their first contribution in https://github.com/flameshot-org/flameshot/pull/2219
* @UnkwUsr made their first contribution in https://github.com/flameshot-org/flameshot/pull/2395
* @ricardovsilva made their first contribution in https://github.com/flameshot-org/flameshot/pull/2518
* @greenfoo made their first contribution in https://github.com/flameshot-org/flameshot/pull/2495
* @zhangfuwen made their first contribution in https://github.com/flameshot-org/flameshot/pull/2520
* @dzg made their first contribution in https://github.com/flameshot-org/flameshot/pull/2566
* @Alaskra made their first contribution in https://github.com/flameshot-org/flameshot/pull/2549
* @vozdeckyl made their first contribution in https://github.com/flameshot-org/flameshot/pull/2552
* @henetiriki made their first contribution in https://github.com/flameshot-org/flameshot/pull/2609

**Full Changelog**: https://github.com/flameshot-org/flameshot/compare/v11.0.0...v12.0.rc1
