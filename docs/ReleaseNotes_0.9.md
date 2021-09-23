# 0.9 Release Notes
Thanks to all the testers and contributors that helped make version 0.9! We are very excited to have improved many
bugs and added new features in version 0.9.


## Known Issues
- Fractional scaling issues are not resolved. We are working with Qt upstream on this issue.


## New Features
- Improved MacOS support. MacOS is now officially supported and we will resolve any reported issues on this platform.

- Thanks to SignPath we are able to offer digitally signed windows releases.
- Improved Wayland support
  - Behind the scenes we configure flameshot to automatically run on xcb. This significantly improves the
    wayland experience. This resolves issues with multimonitor setups and copying to the clipboard

- New option to allow the clipboard image to be a jpeg instead of a png. This may reduce bandwidth when pasting the
image into chat or email clients

- New global shortcut menu. All actions hotkeys are fully customizable.

- Ability to take "symmetric" selections by holding down the Shift key while resizing the selection.

- The rectangle tool will now round the corners of the rectangles based on the current thickness

- All imgur uploads are now tracked in the "Upload History" menu. This makes it much easier to delete of images off imgur
or find the upload link later.

- Added "check for new release" feature. This allows users on MacOS / Windows / and AppImages to easily check for updated versions.

- New option for setting a "fixed save path". When this is enabled a user will no longer need to set the path for images
that are saved.


## Fixes
- Under certain circumstance the circle count could get set to the wrong number with large numbers of undo / redo. This
has been fixed.

- Close after capture has been removed. This feature was not well implemented and lead to numerous bugs.
