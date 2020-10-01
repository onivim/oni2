---
id: settings
title: Settings
sidebar_label: Settings
---

Onivim's settings are designed to be mostly compatible with [VSCode's User Settings](https://code.visualstudio.com/docs/getstarted/settings).

## Editing the user settings

- Press `Command+Shift+P` (Mac) or `Control+Shift+P` (Windows, Linux) to bring up the command palette
- Select `Preferences: Open Configuration File`

### Directly editing the user settings

Sometimes, it is necessary to directly edit the configuration in another editor, for example, if a setting is inadvertently set that makes the editor unusable.

The configuration file, `configuration.json` is in the Oni2 directory, whose location varies by operating system:
- On Unix-like operating systems such as Linux: `$HOME/.config/oni2`
- On Windows, `%LOCALAPPDATA%\Oni2`

## Configuration Settings

### Editor

- `editor.autoClosingBrackets` __(_"LanguageDefined"|"Never"_ default: `"LanguageDefined"`)__ - When set to `"LanguageDefined"`, Onivim will automatically close brackets and pairs, based on language configuration.

- `editor.detectIndentation` __(_bool_ default: `true`)__ - Allow Onivim to auto-detect indentation settings (tab vs space, indent size)

- `editor.fontFamily` __(_string_)__ - The font family used by the editor surface. This must be a monospace font. The font may be specified by either the name of the font, or an absolute path to the font file.

- `editor.fontSize` __(_int_ default: `14`)__ - The font size used by the editor surface.

- `editor.fontSmoothing` __(_"none"|"antialiased"|"subpixel-antialiased"_)__ - The smoothing strategy used when rendering fonts. The `"antialiased"` setting smooths font edges, and `"subpixel-antialiased"` means characters may be positioned fractionally on the pixel grid. 

- `editor.fontLigatures` __(_string|bool_ default: `true`)__ - Sets whether or not font ligatures are enabled. When `true`, the font's default features are enabled. When `false`, contextual alternates and standard ligatues are disabled. If a string is entered, it must be of the form `"'tag1', 'tag2', ..."`, where each tag listed will be enabled. This is particularly useful for enabling stylistic sets, i.e. `"'ss01', 'ss02', ..."`.

- `editor.hover.delay` __(_int_ default: `1000`)__ - The delay in milliseconds before showing the hover UI.

- `editor.hover.enabled` __(_bool_ default: `true`)__ - Controls whether or not the hover UI is enabled.

- `editor.largeFileOptimizations` __(_bool_ default: `true`)__ - When `true`, Onivim will turn off certain settings like syntax highlighting for large files.

- `editor.lineHeight` __(_float_ default: `0.`)__ - Controls the absolute height of lines on the editor surface. Use 0 to compute lineHeight from the font size.

- `editor.lineNumbers` __(_"on"|"off"|"relative"_ default: `"on"`)__ - Controls how line numbers are rendered on the editor surface
    - _"on"_ - absolute line numbers are displayed
    - _"relative"_ - the absolute line number of the line with the cursor is displayed, other line numbers are shown relative. (This is helpful for motions in Vim!)
    - _"off"_ - do not render line numbers.

- `editor.matchBrackets` __(_bool_ default: `true`)__ - Highlight bracket matches on the editor surface.

- `editor.minimapEnabled` __(_bool_ default: `true`)__ - Controls whether or not the minimap is rendered on the editor surface.

- `editor.minimapShowSlider` __(_bool_ default: `true`)__ - Controls where or not a slider representing the current viewport is shown on the minimap.

- `editor.minimapMaxColumn` __(_int_ default: `80`)__ - Sets the maximum column that will be rendered in the minimap. By default, we size the minimap proportionally to the editor surface - this puts a constraint on that size.

- `editor.insertSpaces` __(_bool_ default: `true`)__ - When `true`, the Onivim will use spaces for indentation as opposed to tabs.

- `editor.quickSuggestions` __(_bool_ default: `true`)__ - When `true`, code completions will be enabled. When `false`, code completions will be disabled.

   Code completions also support finer-grained configuration, via a JSON object of the form: `{ "comments": true, "strings": true, "others": true }`.

   This allows enabling code-completions based on the current syntax scope.

- `editor.renderWhitespace` __(_"all"|"boundary"|"selection"|"none"_ default: `"selection"`)__ - Controls how whitespace (tabs/spaces) are rendered:
    - _"all"_ - Render all whitespace
    - _"boundary"_ - Render whitespace except for single characters between text
    - _"selection"_ - Render whitespace characters in visual mode selected text
    - _"none"_ - Don't render whitespace at all

- `editor.rulers` __(_list of int_ default: `[]`)__ - Render vertical rulers at given columns.

- `explorer.autoReveal` __(_string|bool_ default: `true`)__  - When `true`, the file explorer will jump to highlight the file current focused. When `false` the file explorer will remain static. If a string is entered it must be `"focusNoScroll"` which will still highlight the currently focused file in the file explorer but the file explorer will not scroll to it. Any other string supplied will be treated as if `false` was entered and the file explorer will remain static and not highlight the currently focused file. 

- `editor.scrollShadow` __(_bool_ default: `true`)__ - When `true`, show a drop-shadow effect at the borders when there is additional content past the visible area.

- `editor.smoothScroll` __(_bool_ default: `true`)__ - When `true`, smoothly scroll the editor when the viewport is adjusted due to a cursor motion.

- `editor.zenMode.singleFile` __(_bool_ default: `true`)__ - When `true`, the Onivim will automatically enter zen mode when started up with a single file. Zen mode hides most of the UI until disabled via the command pallette.

- `editor.zenMode.hideTabs` __(_bool_ default: `true`)__ - When `true`, the Onivim will hide the buffer tabs from the user whilst in zen mode. Zen mode can be toggled in the command pallette, or automatically enabled with the `editor.zenMode.singleFile` configuration option.

- `workbench.colorTheme` __(_string_ default:`"One Dark Pro"`)__ - Color theme to use.

- `workbench.iconTheme` __(_string_ default: `"vs-seti"`)__ - Icon theme to use.

- `workbench.tree.indent` __(_int_ default: `2`)__ - Indentation of the tree explorer.

- `vim.highlightedyank.enable` __(_bool_ default: `true`)__ - When `true`, briefly highlight yanks on the editor surface.

- `vim.highlightedyank.color` __(_string_)__ - Hex string defining a color, ie `#FF00FFFF`.

- `vim.highlightedyank.duration` __(_int_ default: `300`)__ - The time, in milliseconds, the yank highlight is visible.

## Layout

- `workbench.editor.showTabs` __(_bool_ default: `true`)__ - When `false`, hides the editor tabs.

- `workbench.sideBar.location` __(_"left"|"right"_ default: `"left"`)__ - Controls the location of the sidebar.

- `workbench.sideBar.visible` __(_bool_ default: `true`)__ - Controls the visibility of the sidebar.

- `workbench.statusBar.visible` __(_bool_ default: `true`)__ - Controls the visibility of the status bar.

- `oni.layout.showLayoutTabs` __(_"always"|"smart"|"never"_ default: `"smart"`)__ - Controls the display of layout tabs. `"smart"` will only show the tabs if there's more than one.

- `oni.layout.layoutTabPosition` __(_"top"|"bottom"_ default: `"bottom"`)__ - Controls the position of the layout tabs.

- `oni.layout.singleTabMode` __(_bool_ default: `false`)__ - When `true`, groups will only hold a single editor, and closing this editor will always close the group. It will also hide the editor tabs, and therefore essentially hide the concept of editor groups.

## Rendering

- `vsync` __(_bool_ default: `false`)__ - Whether rendering should sync with vertical retrace of the monitor. VSync adds input latency, as rendering must sync with the refresh rate of the monitor, but it reduces screen tearing.

### UI

- `ui.shadows` __(_bool_ default: `true`)__ - Use drop-shadows in the rendering of menus, overlays, etc.

- `ui.zoom` __(_float_ default: `1.0`)__ - Zoom setting for UI. Factor to scale UI elements. A value of `2.0` will scale the UI by 200%.

- `oni.inactiveWindowOpacity` __(_float_ default: `0.75`)__ - The opacity value, from 0.0 to 1.0, of inactive windows.

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

### Experimental

Experimental features are features that we are working to stabilize and turn on-by-default.

> __NOTE:__ Experimental features may cause instability, like crashes. Use with caution!

- `experimental.editor.cursorSmoothCaretAnimation` - __(_bool_ default: `false`)__ - Use an animation for moving the cursor caret.

- `experimental.editor.smoothScroll` - __(_bool_ default: `false`)__ - Use an animation for scrolling the editor surface.

- `experimental.viml` - __(_string|list of string_ default: `[]`)__ - Execute some VimL upon load. Example: `"experimental.viml": ["nnoremap ; :"]`

> __NOTE:__ The full set and scope of VimL compatibility is not currently tested. We are still working to enable test cases in [`libvim`](https://github.com/onivim/libvim/pull/6). Use at your own risk, in the meantime!
