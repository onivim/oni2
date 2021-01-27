---
id: settings
title: Settings
sidebar_label: Settings
---

Onivim's settings are designed to be mostly compatible with [VSCode's User Settings](https://code.visualstudio.com/docs/getstarted/settings).

## Editing the user settings

-   Press `Command+Shift+P` (Mac) or `Control+Shift+P` (Windows, Linux) to bring up the command palette
-   Select `Preferences: Open Configuration File`

### Directly editing the user settings

Sometimes, it is necessary to directly edit the configuration in another editor, for example, if a setting is inadvertently set that makes the editor unusable.

The configuration file, `configuration.json` is in the Oni2 directory, whose location varies by operating system:

-   On Unix-like operating systems such as Linux: `$HOME/.config/oni2`
-   On Windows, `%LOCALAPPDATA%\Oni2`

## Configuration Settings

### Editor

-   `editor.acceptSuggestionOnEnter` **(_bool_ default: `false`)** - When `true`, the enter key can be used to select a suggestion. By default, the enter key will not be used, so as not to interfere with creating a new line.

-   `editor.autoClosingBrackets` **(_"LanguageDefined"|"Never"_ default: `"LanguageDefined"`)** - When set to `"LanguageDefined"`, Onivim will automatically close brackets and pairs, based on language configuration.

-   `editor.codeLens` **(_bool_ default: `true`)** - Whether to show codelens, when available from a language extension.

-   `editor.cursorSurroundingLines` **(_int_ default: `1`)** - The number of view lines to keep visible above and below the cursor when scrolling. Equivalent to the Vim `scrolloff` setting.

-   `editor.detectIndentation` **(_bool_ default: `true`)** - Allow Onivim to auto-detect indentation settings (tab vs space, indent size)

-   `editor.fontFamily` **(_string_)** - The font family used by the editor surface. This must be a monospace font.

-   `editor.fontWeight` **(_int|string_ 100 | 200 | 300 | 400 | 500 | 600 | 700 | 800 | 900 | "normal" | "bold" default: `"normal"`)** - The font weight used by the editor surface.

-   `editor.fontSize` **(_int_ default: `14`)** - The font size used by the editor surface.

-   `editor.fontSmoothing` **(_"none"|"antialiased"|"subpixel-antialiased"_)** - The smoothing strategy used when rendering fonts. The `"antialiased"` setting smooths font edges, and `"subpixel-antialiased"` means characters may be positioned fractionally on the pixel grid.

-   `editor.fontLigatures` **(_string|bool_ default: `true`)** - Sets whether or not font ligatures are enabled. When `true`, the font's default features are enabled. When `false`, contextual alternates and standard ligatues are disabled. If a string is entered, it must be of the form `"'tag1', 'tag2', ..."`, where each tag listed will be enabled. This is particularly useful for enabling stylistic sets, i.e. `"'ss01', 'ss02', ..."`.

-   `editor.hover.delay` **(_int_ default: `1000`)** - The delay in milliseconds before showing the hover UI.

-   `editor.hover.enabled` **(_bool_ default: `true`)** - Controls whether or not the hover UI is enabled.

-   `editor.largeFileOptimizations` **(_bool_ default: `true`)** - When `true`, Onivim will turn off certain settings like syntax highlighting for large files.

-   `workbench.editor.enablePreview` **(_bool_ default: `true`)** - When `true`, Onivim will open files in _preview mode_ unless a change is made or the tab is double-clicked. In _preview mode_, the editor tab will be re-used.

-   `editor.lineHeight` **(_float_ default: `0.`)** - Controls the absolute height of lines on the editor surface. Use 0 to compute lineHeight from the font size.

-   `editor.lineNumbers` **(_"on"|"off"|"relative"_ default: `"on"`)** - Controls how line numbers are rendered on the editor surface

    -   _"on"_ - absolute line numbers are displayed
    -   _"relative"_ - the absolute line number of the line with the cursor is displayed, other line numbers are shown relative. (This is helpful for motions in Vim!)
    -   _"off"_ - do not render line numbers.

-   `editor.matchBrackets` **(_bool_ default: `true`)** - Highlight bracket matches on the editor surface.

-   `editor.minimapEnabled` **(_bool_ default: `true`)** - Controls whether or not the minimap is rendered on the editor surface.

-   `editor.minimapShowSlider` **(_bool_ default: `true`)** - Controls where or not a slider representing the current viewport is shown on the minimap.

-   `editor.minimapMaxColumn` **(_int_ default: `80`)** - Sets the maximum column that will be rendered in the minimap. By default, we size the minimap proportionally to the editor surface - this puts a constraint on that size.

-   `editor.insertSpaces` **(_bool_ default: `true`)** - When `true`, the Onivim will use spaces for indentation as opposed to tabs.

-   `editor.occurrencesHighlight` **(_bool_ default: `true`)** When `true`, and provided by a language extension, Onivim will highlight all occurrences of the token under the cursor in the active editor.

-   `editor.parameterHints.enabled` **(_bool_ default: `true`)** - When `true`, and provided by a language extension, Onivim will display a pop-up showing hints for the current function invocation.

-   `editor.quickSuggestions` **(_bool_ default: `true`)** - When `true`, code completions will be enabled. When `false`, code completions will be disabled.

    Code completions also support finer-grained configuration, via a JSON object of the form: `{ "comments": true, "strings": true, "others": true }`.

    This allows enabling code-completions based on the current syntax scope.

-   `editor.renderWhitespace` **(_"all"|"boundary"|"selection"|"none"_ default: `"selection"`)** - Controls how whitespace (tabs/spaces) are rendered:

    -   _"all"_ - Render all whitespace
    -   _"boundary"_ - Render whitespace except for single characters between text
    -   _"selection"_ - Render whitespace characters in visual mode selected text
    -   _"none"_ - Don't render whitespace at all

-   `editor.wordBasedSuggestions` **(_bool_ default: `true`)** When `true`, keywords are provided as completion suggestions.

-   `editor.wordWrap` **(_bool_ default: `false`)** When `true`, Onivim will soft-wrap lines at the viewport boundary.

-   `editor.rulers` **(_list of int_ default: `[]`)** - Render vertical rulers at given columns.

-   `explorer.autoReveal` **(_string|bool_ default: `true`)** - When `true`, the file explorer will jump to highlight the file current focused. When `false` the file explorer will remain static. If a string is entered it must be `"focusNoScroll"` which will still highlight the currently focused file in the file explorer but the file explorer will not scroll to it. Any other string supplied will be treated as if `false` was entered and the file explorer will remain static and not highlight the currently focused file.

-   `editor.scrollShadow` **(_bool_ default: `true`)** - When `true`, show a drop-shadow effect at the borders when there is additional content past the visible area.

-   `editor.smoothScroll` **(_bool_ default: `true`)** - When `true`, smoothly scroll the editor when the viewport is adjusted due to a cursor motion.

-   `editor.zenMode.singleFile` **(_bool_ default: `true`)** - When `true`, the Onivim will automatically enter zen mode when started up with a single file. Zen mode hides most of the UI until disabled via the command pallette.

-   `editor.zenMode.hideTabs` **(_bool_ default: `true`)** - When `true`, the Onivim will hide the buffer tabs from the user whilst in zen mode. Zen mode can be toggled in the command pallette, or automatically enabled with the `editor.zenMode.singleFile` configuration option.

-   `files.exclude` **(_list of string_ default: `[".git", "_esy", "node_modules"]`)** - When using `Quick Open` or `Find in files`, Onivim will ignore the files inside the directories listed here

-   `search.exclude` **(_list of string_ default: `[]`)** - When using `Find in files` Onivim will not look at files located at the directories listed here, this inherit all the values from `files.exclude`

-   `workbench.colorTheme` **(_string_ default:`"One Dark Pro"`)** - Color theme to use.

-   `workbench.iconTheme` **(_string_ default: `"vs-seti"`)** - Icon theme to use.

-   `workbench.tree.indent` **(_int_ default: `2`)** - Indentation of the tree explorer.

-   `vim.highlightedyank.enable` **(_bool_ default: `true`)** - When `true`, briefly highlight yanks on the editor surface.

-   `vim.highlightedyank.color` **(_string_)** - Hex string defining a color, ie `#FF00FFFF`.

-   `vim.highlightedyank.duration` **(_int_ default: `300`)** - The time, in milliseconds, the yank highlight is visible.

### Input

-   `vim.leader` **(_string_)** - Specify a custom [leader key](./key-bindings#leader-key).

### Layout

-   `workbench.editor.showTabs` **(_bool_ default: `true`)** - When `false`, hides the editor tabs.

-   `workbench.sideBar.location` **(_"left"|"right"_ default: `"left"`)** - Controls the location of the sidebar.

-   `workbench.sideBar.visible` **(_bool_ default: `true`)** - Controls the visibility of the sidebar.

-   `workbench.statusBar.visible` **(_bool_ default: `true`)** - Controls the visibility of the status bar.

-   `window.menuBarVisibility` **(_"visible" | "hidden"_ default: `"visible"`)** - Controls the visibility of the menu bar.

-   `oni.layout.showLayoutTabs` **(_"always"|"smart"|"never"_ default: `"smart"`)** - Controls the display of layout tabs. `"smart"` will only show the tabs if there's more than one.

-   `oni.layout.layoutTabPosition` **(_"top"|"bottom"_ default: `"bottom"`)** - Controls the position of the layout tabs.

-   `oni.layout.singleTabMode` **(_bool_ default: `false`)** - When `true`, groups will only hold a single editor, and closing this editor will always close the group. It will also hide the editor tabs, and therefore essentially hide the concept of editor groups.

### Rendering

-   `vsync` **(_bool_ default: `false`)** - Whether rendering should sync with vertical retrace of the monitor. VSync adds input latency, as rendering must sync with the refresh rate of the monitor, but it reduces screen tearing.

### UI

-   `ui.shadows` **(_bool_ default: `true`)** - Use drop-shadows in the rendering of menus, overlays, etc.

-   `ui.zoom` **(_float_ default: `1.0`)** - Zoom setting for UI. Factor to scale UI elements. A value of `2.0` will scale the UI by 200%.

-   `oni.inactiveWindowOpacity` **(_float_ default: `0.75`)** - The opacity value, from 0.0 to 1.0, of inactive windows.

### Vim

-   `vim.useSystemClipboard` **(_`true`_|_`false`_|_`["yank", "paste", "delete"]`_ default: `["yank"]`)** - Whether or not deletes / yanks should integrate with the system clipboard:
    -   _`true`_ - all deletes and yanks, regardless of register used, will be pushed to the system clipboard. Equivalent to `["yank", "paste", "delete"]`.
    -   _`["yank", "paste", "delete"]`_ - An array of strings. Each specified operation will always use the system clipboard. For example, `["yank"]` will send all yanks to the system clipboard, but deletes and pastes will require using the `+` or `*` registers. `["delete", "paste"]` means that all deletes will be sent to the system clipboard, and pastes using the unnamed register will come from the system clipboard, but only yanks with register `+` and `*` would be sent to the clipboard.
    -   _`false`_ - only deletes / yanks using the `+` or `*` registers will be pushed to the system clipboard. Equivalent to `[]`.

### High-DPI / UI Scaling

Onivim 2 should automatically pick up your scaling settings via the following per-platform strategies:

-   **Windows:** On 8.1+, we use the 'scale factor' of the display device.
-   **OSX:** - High-DPI / retina displays are automatically detected.
-   **Linux:** - The `GDK_SCALE` environment variable is used, if available.

If the display scaling is not correct, you can override by using the `--force-device-scale-factor` command-line argument, like:

```
oni2 --force-device-scale-factor=2.0
```

### Experimental

Experimental features are features that we are working to stabilize and turn on-by-default.

> **NOTE:** Experimental features may cause instability, like crashes. Use with caution!

-   `experimental.editor.cursorSmoothCaretAnimation` - **(_bool_ default: `false`)** - Use an animation for moving the cursor caret.

-   `experimental.viml` - **(_string|list of string_ default: `[]`)** - Execute some VimL upon load. Example: `"experimental.viml": ["nnoremap ; :"]`

> **NOTE:** The full set and scope of VimL compatibility is not currently tested. We are still working to enable test cases in [`libvim`](https://github.com/onivim/libvim/pull/6). Use at your own risk, in the meantime!

## Language-specific configuration

Configuration can be specified per-filetype, by specifying a filetype in the `configuration.json`, ie:

```
{
    "editor.insertSpaces": true,
    "[reason]": {
        "editor.detectIndentation": false,
        "editor.insertSpaces": false,
    }
}
```

In the above example, the `editor.insertSpaces` value of `false` in the `reason` section overrides the default of `true` - this configures the editor to use tabs in reason files for indentation, and spaces elsewhere.
