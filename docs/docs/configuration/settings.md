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

- `editor.acceptSuggestionOnEnter` __(_bool_ default: `true`)__ - When `true`, the enter key can be used to accept a suggestion. Users may wish to set to `false` to avoid a conflict with inserting a new line.

- `editor.acceptSuggestionOnTab` __(_bool_ default: `true`)__ - When `true`, the tab key can be used to accept a suggestion. Users may wish to turn to `false` to avoid ambiguity with inserting a tab character.

- `editor.autoClosingBrackets` __(_"LanguageDefined"|"Never"_ default: `"LanguageDefined"`)__ - When set to `"LanguageDefined"`, Onivim will automatically close brackets and pairs, based on language configuration.

- `editor.codeLens` __(_bool_ default: `true`)__ - Whether to show codelens, when available from a language extension.

- `editor.cursorSurroundingLines` __(_int_ default: `1`)__ - The number of view lines to keep visible above and below the cursor when scrolling. Equivalent to the Vim `scrolloff` setting.

- `editor.detectIndentation` __(_bool_ default: `true`)__ - Allow Onivim to auto-detect indentation settings (tab vs space, indent size)

- `editor.fontFamily` __(_string_)__ - The font family used by the editor surface. This must be a monospace font.

- `editor.fontWeight` __(_int|string_ 100 | 200 | 300 | 400 | 500 | 600 | 700 | 800 | 900 | "normal" | "bold" default: `"normal"`)__ - The font weight used by the editor surface.

- `editor.fontSize` __(_int_ default: `14`)__ - The font size used by the editor surface.

- `editor.fontSmoothing` __(_"none"|"antialiased"|"subpixel-antialiased"_ default: `"subpixel-antialiased"`)__ - The smoothing strategy used when rendering fonts. The `"antialiased"` setting smooths font edges, and `"subpixel-antialiased"` means characters may be positioned fractionally on the pixel grid.

- `editor.fontLigatures` __(_string|bool_ default: `true`)__ - Sets whether or not font ligatures are enabled. When `true`, the font's default features are enabled. When `false`, contextual alternates and standard ligatues are disabled. If a string is entered, it must be of the form `"'tag1', 'tag2', ..."`, where each tag listed will be enabled. This is particularly useful for enabling stylistic sets, i.e. `"'ss01', 'ss02', ..."`.

- `editor.hover.delay` __(_int_ default: `1000`)__ - The delay in milliseconds before showing the hover UI.

- `editor.hover.enabled` __(_bool_ default: `true`)__ - Controls whether or not the hover UI is enabled.

- `editor.largeFileOptimizations` __(_bool_ default: `true`)__ - When `true`, Onivim will turn off certain settings like syntax highlighting for large files.

- `editor.showDeprecated` __(_bool_ default: `true`)__ - When `true`, Onivim will render deprecated code with a strike-through.

- `editor.showUnused` __(_bool_ default: `true`)__ - When `true`, Onivim will fade out unused code.

- `workbench.editor.enablePreview` __(_bool_ default: `true`)__ - When `true`, Onivim will open files in _preview mode_ unless a change is made or the tab is double-clicked. In _preview mode_, the editor tab will be re-used.

- `editor.lightBulb.enabled` __(_bool_ default: `true`)__ - When `true`, show a lightbulb icon in the editor if there are quick fixes or refactorings available.

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

- `editor.occurrencesHighlight` __(_bool_ default: `true`)__ When `true`, and provided by a language extension, Onivim will highlight all occurrences of the token under the cursor in the active editor.

- `editor.parameterHints.enabled` __(_bool_ default: `true`)__ - When `true`, and provided by a language extension, Onivim will display a pop-up showing hints for the current function invocation.

- `editor.quickSuggestions` __(_bool_ default: `true`)__ - When `true`, code completions will be enabled. When `false`, code completions will be disabled.

   Code completions also support finer-grained configuration, via a JSON object of the form: `{ "comments": true, "strings": true, "others": true }`.

   This allows enabling code-completions based on the current syntax scope.

- `editor.renderWhitespace` __(_"all"|"boundary"|"selection"|"none"_ default: `"selection"`)__ - Controls how whitespace (tabs/spaces) are rendered:
    - _"all"_ - Render all whitespace
    - _"boundary"_ - Render whitespace except for single characters between text
    - _"selection"_ - Render whitespace characters in visual mode selected text
    - _"none"_ - Don't render whitespace at all

- `editor.scrollbar.horizontalScrollbarSize` __(_int_ default: `8`)__ - The size, in pixels, of the horizontal scroll bar on the editor surface.

- `editor.scrollbar.verticalScrollbarSize` __(_int_ default: `15`)__ - The size, in pixels, of the vertical scroll bar on the editor surface.

- `editor.snippetSuggestions` __(_string_ default: `"inline"`) - controls how snippets are presented in the suggestion UI:
    - _"top"_ - Show snippets at the top of the suggestion list
    - _"bottom"_ - Show snippets at the bottom of the suggestion list
    - _"inline"_ - Show snippets sorted in-line with other suggestion items
    - _"hidden"_ - Don't show snippet suggestions at all

- `editor.suggest.itemsToShow` __(_int_ default: `5`)__ - The maximum number of suggestions to show in the suggestion UI.

- `editor.wordBasedSuggestions` __(_bool_ default: `true`)__ When `true`, keywords are provided as completion suggestions.

- `editor.wordWrap` __(_bool_ default: `true`)__ When `true`, Onivim will soft-wrap lines at the viewport boundary.

- `editor.rulers` __(_list of int_ default: `[]`)__ - Render vertical rulers at given columns.

- `editor.scrollShadow` __(_bool_ default: `true`)__ - When `true`, show a drop-shadow effect at the borders when there is additional content past the visible area.

- `editor.smoothScroll` __(_bool_ default: `true`)__ - When `true`, smoothly scroll the editor when the viewport is adjusted due to a cursor motion.

- `editor.zenMode.singleFile` __(_bool_ default: `true`)__ - When `true`, the Onivim will automatically enter zen mode when started up with a single file. Zen mode hides most of the UI until disabled via the command pallette.

- `editor.zenMode.hideTabs` __(_bool_ default: `true`)__ - When `true`, the Onivim will hide the buffer tabs from the user whilst in zen mode. Zen mode can be toggled in the command pallette, or automatically enabled with the `editor.zenMode.singleFile` configuration option.

- `files.exclude` __(_list of string_ default: `[".git", "_esy", "node_modules"]`)__ - When using `Quick Open` or `Find in files`, Onivim will ignore the files inside the directories listed here 

- `files.autoSave` __(_string_ default: `"off"`)__ - controls when buffers are auto-saved:
    - _"off"_ - Do not auto-save at all
    - _"afterDelay"_ - Auto-save after the delay specified by `"files.autoSaveDelay"`
    - _"onFocusChange"_ - Auto-save when changing focus between buffers
    - _"onWindowChange"_ - Auto-save when the Onivim application window loses focus

- `files.autoSaveDelay` __(_int_ default: `1000`)__ - specifies the time, in milliseconds, to wait to auto-save a buffer when `files.autoSave` is set to `"afterDelay"`

- `search.exclude` __(_list of string_ default: `[]`)__ - When using `Find in files` Onivim will not look at files located at the directories listed here, this inherit all the values from `files.exclude`

- `search.followSymlinks` __(_bool_ default:  `true`)__ - Set whether to follow symlinks when searching

- `search.useIgnoreFiles` __(_bool_ default:  `true`)__ - Set whether to `.gitignore` should be respected when searching

- `workbench.colorCustomizations` __(_json_ default: `{}`)__ - Color theme overrides, using the same [Theme Colors as Code](https://code.visualstudio.com/api/references/theme-color) - for example:

```json
  "workbench.colorCustomizations": {
    "terminal.background": "#0F0",
    "terminal.foreground": "#FFF"
  },
```

- `workbench.colorTheme` __(_string_ default:`"One Dark Pro"`)__ - Color theme to use.

- `workbench.iconTheme` __(_string_ default: `"vs-seti"`)__ - Icon theme to use.

- `workbench.tree.indent` __(_int_ default: `5`)__ - Indentation of the tree explorer.

- `workbench.tree.renderIndentGuides` __(_bool_ default: `true`)__ - Controls whether indent guide lines are rendered in tree views.

- `vim.highlightedyank.enable` __(_bool_ default: `true`)__ - When `true`, briefly highlight yanks on the editor surface.

- `vim.highlightedyank.color` __(_string_)__ - Hex string defining a color, ie `#FF00FFFF`.

- `vim.highlightedyank.duration` __(_int_ default: `300`)__ - The time, in milliseconds, the yank highlight is visible.

### Input

- `vim.leader` __(_string_)__ - Specify a custom [leader key](./key-bindings#leader-key).

- `vim.timeout` __(_int_ default: `1000`)__ Sets the timeout, in milliseconds, when Onivim is waiting for a pending chord. When the timeout is reached, any pending keys that are partially mapped will be flushed. Equivalent to the `timeoutlen` Vim setting. Can be set to `0` to disable the timeout entirely.

### Explorer

- `explorer.autoReveal` __(_string|bool_ default: `true`)__  - When `true`, the file explorer will jump to highlight the file current focused. When `false` the file explorer will remain static. If a string is entered it must be `"focusNoScroll"` which will still highlight the currently focused file in the file explorer but the file explorer will not scroll to it. Any other string supplied will be treated as if `false` was entered and the file explorer will remain static and not highlight the currently focused file.

- `files.useExperimentalFileWatcher` __(_bool_ default: `true`)__ When `true`, a file watcher will be used to monitor file system changes and update the explorer in the sidebar.

### Layout

- `workbench.activityBar.visible` __(_bool_ default: `true`)__ - Controls whether or not the activity bar (icon bar) is visible.

- `workbench.editor.showTabs` __(_bool_ default: `true`)__ - When `false`, hides the editor tabs.

- `workbench.sideBar.location` __(_"left"|"right"_ default: `"left"`)__ - Controls the location of the sidebar.

- `workbench.sideBar.visible` __(_bool_ default: `true`)__ - Controls the visibility of the sidebar.

- `workbench.statusBar.visible` __(_bool_ default: `true`)__ - Controls the visibility of the status bar.

- `window.menuBarVisibility` __(_"visible" | "hidden"_ default: `"visible"`)__ - Controls the visibility of the menu bar.

- `window.titleBarStyle` __(_"native" | "custom"_ default: `"native"` on Windows, `"custom"` otherwise)__ - Controls whether the titlebar is custom-rendered.

- `oni.layout.showLayoutTabs` __(_"always"|"smart"|"never"_ default: `"smart"`)__ - Controls the display of layout tabs. `"smart"` will only show the tabs if there's more than one.

- `oni.layout.layoutTabPosition` __(_"top"|"bottom"_ default: `"bottom"`)__ - Controls the position of the layout tabs.

- `oni.layout.singleTabMode` __(_bool_ default: `false`)__ - When `true`, groups will only hold a single editor, and closing this editor will always close the group. It will also hide the editor tabs, and therefore essentially hide the concept of editor groups.

### Proxy

Onivim 2 can be configured to send requests through an HTTP/HTTPs proxy with the following configuration:

- `http.proxy` __(_string_ default: `null`) - A URL to be used as a proxy server, including the name and password. ie, `"http.proxy": "http://user@pass:127.0.0.1:8888"`

- `https.proxy` - __(_string_ default: `null`) - A URL to be used as a proxy server for HTTPs requests. If not specified, Onivim will fall back to the `http.proxy` setting.

### Rendering

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

### High-DPI / UI Scaling

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

- `experimental.viml` - __(_string|list of string_ default: `[]`)__ - Execute some VimL upon load. Example: `"experimental.viml": ["nnoremap ; :"]`

> __NOTE:__ The full set and scope of VimL compatibility is not currently tested. We are still working to enable test cases in [`libvim`](https://github.com/onivim/libvim/pull/6). Use at your own risk, in the meantime!

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
