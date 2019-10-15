---
id: configuration
title: Configuration
sidebar_label: Configuration
---

Onivim's configuration is designed to be mostly compatible with [VSCode's User Settings](https://code.visualstudio.com/docs/getstarted/settings).

## Editing the user configuration

- Press `Command+Shift+P` (Mac) or `Control+Shift+P` (Windows, Linux) to bring up the command palette
- Select `Preferences: Open Configuration File`

## Configuration Settings

### Editor

- `editor.detectIndentation` __(_bool_ default: `true`)__ - Allow Onivim to auto-detect indentation settings (tab vs space, indent size)

- `editor.fontFamily` __(_string_)__ - The font family used by the editor surface. This must be a monospace font. The font may be specified by either the name of the font, or an absolute path to the font file.

- `editor.fontSize` __(_int_ default: `14`)__ - The font size used by the editor surface.

- `editor.largeFileOptimizations` __(_bool_ default: `true`)__ - When `true`, Onivim will turn off certain settings like syntax highlighting for large files.

- `editor.lineNumbers` __(_"on"|"off"|"relative" _default: `"on"`)__ - Controls how line numbers are rendered on the editor surface
    - _"on"_ - absolute line numbers are displayed
    - _"relative"_ - the absolute line number of the line with the cursor is displayed, other line numbers are shown relative. (This is helpful for motions in Vim!)
    - _"off"_ - do not render line numbers.

- `editor.matchBrackets` __(_bool_ default: `true`)__ - Highlight bracket matches on the editor surface.

- `editor.minimapEnabled` __(_bool_ default: `true`)__ - Controls whether or not the minimap is rendered on the editor surface.

- `editor.minimapShowSlider` __(_bool_ default: `true`)__ - Controls where or not a slider representing the current viewport is shown on the minimap.

- `editor.minimapMaxColumn` __(_int_ default: `80`)__ - Sets the maximum column that will be rendered in the minimap. By default, we size the minimap proportionally to the editor surface - this puts a constraint on that size.

- `editor.insertSpaces` __(_bool_ default: `true`)__ - When `true`, the Onivim will use spaces for indentation as opposed to tabs.

- `editor.rulers` __(_list of int_ default: `[]`)__ - Render vertical rulers at given columns.

- `workbench.colorTheme` __(_string)_ default:`"One Dark Pro"`)__ - Color theme to use.

- `workbench.tree.indent` __(_int_ default: `2`)__ - Indentation of the tree explorer.

### UI

- `ui.shadows` __(_bool_ default: `true`)__ - Use drop-shadows in the rendering of menus, overlays, etc.

- `ui.zoom` __(_float_ default: `1.0`)__ - Zoom setting for UI. Factor to scale UI elements. A value of `2.0` will scale the UI by 200%.

### Vim

- `vim.useSystemClipboard` __(_`true`_|_`false`_|_`["yank", "paste", "delete"]`_ default: `["yank"]`)__ - Whether or not deletes / yanks should integrate with the system clipboard:
    - _`true`_ - all deletes and yanks, regardless of register used, will be pushed to the system clipboard. Equivalent to `["yank", "paste", "delete"]`.
    - _`["yank", "paste", "delete"]`_ - An array of strings. Each specified operation will always use the system clipboard. For example, `["yank"]` will send all yanks to the system clipboard, but deletes and pastes will require using the `+` or `*` registers. `["delete", "paste"]` means that all deletes will be sent to the system clipboard, and pastes using the unnamed register will come from the system clipboard, but only yanks with register `+` and `*` would be sent to the clipboard.
    - _`false`_ - only deletes / yanks using the `+` or `*` registers will be pushed to the system clipboard. Equivalent to `[]`.

## High-DPI / UI Scaling

Onivim 2 should automatically pick up your scaling settings via the following per-platform strategies:

- __Windows:__ On 8.1+, we use the 'scale factor' of the display device.
- __OSX:__ - High-DPI / retina displays are automatically detected.
- __Linux:__ - The `GDK_SCALE` environment variable is used, if available.

If the display scaling is not correct, you can override by using the `--force-device-scale-factor` command-line argument, like:

```
oni2 --force-device-scale-factor=2.0
```

> __NOTE:__ Due to a [current limitation in Revery](https://github.com/revery-ui/revery/issues/598), fractional scaling is not yet supported
