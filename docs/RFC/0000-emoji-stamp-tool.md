# Add an emoji stamp tool

### To be reviewed by

* mmahmoudian
* Flameshot developers

<br>

### Authors

* Damien Degois (@babs)

<br>

### Status: Draft ~~| Discussion | Active | Dropped | Superseded~~

<br>

### Superseded by

N/A

<br>

### Related

* Issue #764, "Tool to allow adding emojis to the capture"
* Issue #4656, "When making screenshot have a tool to add icons before saving the screenshot"
* Working implementation: https://github.com/babs/flameshot-org-flameshot/tree/feat/emoji-stamp-tool

<br>

## Problem

People mark up screenshots with emojis. A check mark on the step that worked, a cross on the one that failed, a pointing hand or a pair of eyes on the part that matters. Flameshot has no tool for this.

The workaround quoted in #764 is to paste an emoji into the text tool. That gives one emoji per clipboard copy, and the text font often draws it in monochrome or not at all.

#764 has been open since June 2020. In that thread @mmahmoudian described the design this RFC follows. Clicking the tool opens the side panel with a list of emojis and a search box, and the user places them and sets their size the same way as the circle counter.

<br>

## Anti-Goals

* Image stickers. Loading PNG packs, as ShareX does, needs its own file handling and deserves its own proposal. See Prior art.
* Shipping an emoji font. The tool draws with the color emoji font the system already has.
* Skin tone selection and a recently used row. Both are listed under Future work.

<br>

## Solution

A new tool, "Emoji", sits in the toolbar right after the circle counter.

### Placing

Selecting the tool opens the side panel on the picker. The chosen emoji follows the cursor as a half transparent preview, and a click stamps it. The mouse wheel sets the size on the same scale as the circle counter bubble. A stamp is a regular layer, so it can be selected, moved, deleted, undone and redone.

### Picker

The panel holds a search field, a category list and a scrollable grid.

* Search matches English Unicode names by substring. Typing "upside" finds 🙃. Return picks the first result.
* The category list follows the Unicode groups: Smileys & Emotion, People & Body, Animals & Nature, Food & Drink, Travel & Places, Activities, Objects, Symbols and Flags.
* An emoji pasted into the search field is used as is, whether it comes from the clipboard or an input method picker. This is how skin tone variants get in, since the grid leaves them out.
* Hovering a cell shows its name.
* Picking an emoji hands the keyboard back to the editor, so Ctrl+C, Ctrl+Z and the tool keys work right away. Escape in the search field leaves the field instead of closing the capture.

### Editing

Selecting a placed emoji, on the capture or in the layer list, shows the picker in the side panel. A pick replaces the selected emoji. Several picks in a row count as one change, so one Ctrl+Z brings back the original. The keyboard stays on the canvas, so arrow keys still move the selected emoji.

### Shortcut

Ctrl+. selects the tool. Nothing in Flameshot uses it today, and it stays configurable like every other tool shortcut. The README table and the example config list it.

### Data

Names and groups come from Unicode's `emoji-test.txt`, embedded as a Qt resource next to the Unicode license. When the panel opens, the picker keeps only the sequences the platform emoji font can draw, so the grid never shows a missing glyph box. On Ubuntu 24.04 with Noto Color Emoji that leaves 1,914 emojis.

### Font

Drawing asks for Noto Color Emoji, then Apple Color Emoji, then Segoe UI Emoji. Qt uses the first one installed.

### Code footprint

* One new directory, `src/tools/emoji/`, with the tool and its picker split the way the text tool splits `TextTool` and `TextConfig`.
* The usual registration points for a new tool: the tool factory, both button lists, the shortcut table and the resource file.
* A few changes in `CaptureWidget`. Selecting the tool opens the side panel. Selecting a placed emoji shows the picker, and a pick goes through the undo handling that resizing a selected object already uses.
* `undo()` and `redo()` now record a pending change of the selected object before they run. Without this, Ctrl+Z right after resizing a selected object skipped the resize and undid the action before it. Replacing an emoji would have inherited the same defect.

<br>

## Performance Impact

Opening the panel parses the embedded list and checks each code point against the font. That takes about 13 ms. Filtering and rebuilding the grid takes about 2 ms per keystroke. Both figures come from a PyQt6 replica of the same logic on Qt 6.9 under Linux, not from the C++ build.

Stamping costs one `drawText` call per emoji.

The resource adds about 65 KB to the binary after rcc compression. The text file adds 670 KB to the repository.

<br>

## Backwards Compatibility and Upgrade Path

* The new tool type is appended to the end of the enum with value 25, so saved configurations keep their meaning.
* Users who never customized their button list get the new button. Users who did find it unchecked in the settings, the same as with any new tool.
* No file format or command line flag changes.

<br>

## Drawbacks

* **The 670 KB data file.** Keeping only code points, names and groups would bring it near 100 KB. The file would then no longer be Unicode's verbatim copy, and each Unicode release would need a conversion step instead of a plain file swap. I kept it verbatim and will follow the team's preference.
* **Color depends on the platform font.** Linux distributions ship Noto Color Emoji and macOS ships Apple Color Emoji. I have not yet checked that Qt draws Segoe UI Emoji in color on Windows. With no emoji font at all, the panel says "No emoji font found" and the grid is disabled.
* **English names only.** Unicode's CLDR annotations would allow localized search, at the cost of one more data file per language.
* **The tool opens the side panel.** No other tool does this. The picker lives in the panel, so leaving it closed would give the user no way to choose.

<br>

## Prior Art

* @borgmanJeremy's prototype linked in #764, https://github.com/borgmanJeremy/emoji, drew one image file per emoji and had categories and skin tones. Work stopped when he left the project. Images render the same on every platform, but they mean shipping and licensing a full set such as Twemoji or Noto, which weighs several MB.
* ShareX ships image sticker packs. @OptionalM asked for the same in #764. Stickers could reuse the placement code of this tool later.
* The GNOME Emoji Selector extension, cited in #764 by @nettum, offers search, a recent row and categories. This RFC covers search and categories.
* Pasting into the text tool stays available and keeps its current limits.

<br>

## Future work

* A recently used row at the top of the grid.
* Skin tone selection, as asked in #764.
* Localized search from CLDR annotations.

<br>

## FAQ

None yet.

<br>

## Errata

None yet.
