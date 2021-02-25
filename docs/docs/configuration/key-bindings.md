---
id: key-bindings
title: Key Bindings
sidebar_label: Key Bindings
---

Onivim's keyboard configuration is designed to be mostly compatible with [VSCode's Key Bindings](https://code.visualstudio.com/docs/getstarted/keybindings).

## Editing the keybindings

- Press `Command+Shift+P` (Mac) or `Control+Shift+P` (Windows, Linux) to bring up the command palette
- Select `Preferences: Open keybindings File`

### Keybinding Format

Keybindings are defined as a JSON array, like:
```
[
  { "key": "<C-P>", "command": "workbench.action.quickOpen", "when": "editorTextFocus" }
]
```

Each rule consists of:
- A `key` that describes the pressed keys

> __NOTE:__ Today, our key format is expressed as a Vim-style key-binding. Our plan, though, is to support both Vim-style (`<C-K>`) and VSCode-style key specifies (`ctrl+k`)

- A `command` containing the identifier of the command to execute
- An __optional__ `when` clause containing a boolean expression that will be evaluated depending on the current editor state.

When a key is pressed:
- The rules are evaluated from __bottom__ to __top__
- The first rule that matches, both the `key` and in terms of `when` is accepted
- If a rule is found and has a `command` set, the `command` is executed.
- If no matching rules are found, we pass the input key through to `libvim` to be handled by Vim.

There are a set of default rules provided by Onivim, but the customized rules are appended to the bottom - thus, user key bindings are executed first.

### `key` format

The `key` parameter supports both _Vim style_ and _VSCode style_ key bindings.

#### Vim style

Vim-style keybindings are surrounded by `<` and `>`, and allow the following modifiers:

| Modifier | Description | Example |
| --- | --- | --- |
| `C-` | Control key | `<C-P>` |
| `S-` | Shift key | `<S-P>` |
| `A-` | Alt key | `<A-P>` |
| `D-` | Command key | `<D-P>` | 

> __Note:__ A difference between Vim and Onivim is that `D-` also handles the 'Meta' and 'Win' keys on Linux and Windows, respectively

Modifiers may be combined, for example:

```
[
  { "key": "<C-S-P>", "command": "workbench.action.quickOpen", "when": "editorTextFocus" }
]
```

The `<C-S-P>` key binding would require the Control, Shift, and P keys to be pressed.

#### VSCode style

VSCode-style keybindings feature friendly names, like:

| Modifier | Description | Example |
| --- | --- | --- |
| `Ctrl+` | Control key | `Ctrl+P` |
| `Shift+` | Shift key | `Shift+P` |
| `Alt+` | Alt key | `Alt+J` |
| `Meta+` | Meta/Command/Windows key |
| `Cmd+` | Same as above | `Cmd+P` |
| `Win+` | Same as above | `Win+P` |

### Key Sequences

Onivim supports binding to key sequences, which require multiple key-presses in succession to engage.

Example:

```
  { "key": "jk", "command": "vim.esc", "when": "insertMode" }
```

This would require a key-press of 'j', followed by 'k'.


### `command` arguments

### `when` clause contexts

Onivim 2 gives you control over when key bindings are enabled through the `when` clause. 

#### Conditional operators

For conditional expressions, you can use the following conditional operators:

| Operator | Symbol | Example |
| --- | --- | --- |
| Or | <code>&#124;&#124;</code> | <code>listFocus &#124;&#124; textInputFocus</code> |
| And | `&&` | `insertMode && suggestWidgetVisible` |

Expressions may be grouped with `()`, for example:
```
(editorTextFocus && !insertMode) || suggestWidgetVisible
```

#### Contexts

Common contexts with VSCode:

| Context Name | True When | 
| --- | --- |
| `editorTextFocus` | An editor has focus |
| `inSnippetMode` | A snippet session is currently active |
| `textInputFocus` | A text input area has focus |
| `terminalFocus` | A terminal has focus |
| `suggestWidgetVisible` | The suggest widget (auto-completion) is visible |

Onivim-specific contexts:

| Context Name | True When | 
| --- | --- |
| `insertMode` |  The active editor is in `insert` mode |
| `normalMode` |  The active editor is in `normal` mode |
| `visualMode` |  The active editor is in `visual` mode |
| `sneakMode` | Sneak mode is active |
| `commandLineFocus` | The Vim commandline is open |
| `listFocus` | A list of items (like a pop-up menu) is focused |
| `vimListNavigation` | Inside a Vim-navigable list |
| `vimTreeNavigation` | Inside the file explorer |

## Commands

| Default Key Binding | Description | Command |
| --- | --- | --- |
| Command+Shift+P / Control+Shift+P | Show Command Palette | `workbench.action.showCommands` |
| Command+Shift+M / Control+Shift+M | Show Problems Pane | `workbench.actions.view.problems` |
| Command+P / Control+P | Quick Open (File Picker) | `workbench.action.quickOpen` | 
| Control+Tab | Navigate to next editor in group | `workbench.action.quickOpenNavigateNextInEditorPicker` |
| Shift+Control+Tab | Navigate to previous editor in group | `workbench.action.quickOpenNavigatePreviousInEditorPicker` | 
| Command+= / Control+= | Zoom In | `workbench.action.zoomIn` | 
| Command+- / Control+- | Zoom Out | `workbench.action.zoomOut` | 
| Command+0 / Control+0 | Zoom Reset | `workbench.action.zoomReset` | 

### Basic Editing

| Default Key Binding | Description | Command |
| --- | --- | --- |
| Control+V / Command+V | Paste from clipboard | `editor.action.clipboardPasteAction` |
| Control+S / Command+S | Save file | `workbench.action.files.save`

### List / Menu commands

| Default Key Binding | Description | Command |
| --- | --- | --- |
| Up Arrow / Control+P | Move focus up | `list.focusUp` |
| Down Arrow / Control+N | Move focus down | `list.focusDown` |

### Sidebar

| Default Key Binding | Description | Command |
| --- | --- | --- |
| Command+Shift+E / Control+Shift+E | Focus Explorer | `workbench.view.explorer` |
| Command+Shift+F / Control+Shift+F | Focus Search | `workbench.action.findInFiles` |
| Command+Shift+X / Control+Shift+X | Focus Extensions | `workbench.view.extensions` |
| Control+Shift+G | Focus Source Control | `workbench.view.scm` |
| Control+W, Control+J | Navigate down a section | `vim.window.moveDown` |
| Control+W, Control+K | Navigate up a section | `vim.window.moveUp` |

### Snippets

| Default Key Binding | Description | Command | 
| --- | --- | --- |
| n/a | Insert a Snippet | `editor.action.insertSnippet` |
| n/a | Configure user snippets | `workbench.action.openSnippets` |

### Window Management

| Default Key Binding | Description | Command |
| --- | --- | --- |
| Control+W, Control+V | Vertical Split | `view.splitVertical` |
| Control+W, Control+S | Horizontal Split | `view.splitHorizontal` |
| Control+W, Control+H | Move to left split | `window.moveLeft` |
| Control+W, Control+L | Move to right split | `window.moveRight` |
| Control+W, Control+J | Move down a split | `window.moveDown` |
| Control+W, Control+K | Move up a split | `window.moveUp` |
| Control+W, Control+R | Rotate forwards | `view.rotateForward` |
| ~~Control+W, Control+Shift+R~~ | Rotate backwards | `view.rotateBackward` |
| | Decrease editor size | `workbench.action.decreaseViewSize` |
| | Increase editor size | `workbench.action.increaseViewSize` |
| ~~Control+W, <~~ | Decrease editor size horizontally | `vim.decreaseHorizontalWindowSize` |
| ~~Control+W, >~~ | Increase editor size horizontally | `vim.increaseHorizontalWindowSize` |
| Control+W, - | Decrease editor size vertically | `vim.decreaseVerticalWindowSize` |
| ~~Control+W, +~~ | Increase editor size vertically | `vim.increaseVerticalWindowSize` |
| | Decrease editor size up | `vim.decreaseWindowSizeUp` |
| | Increase editor size up | `vim.increaseWindowSizeUo` |
| | Decrease editor size down | `vim.decreaseWindowSizeDown` |
| | Increase editor size down | `vim.increaseWindowSizeDown` |
| | Decrease editor size left | `vim.decreaseWindowSizeLeft` |
| | Increase editor size left | `vim.increaseWindowSizeLeft` |
| | Decrease editor size right | `vim.decreaseWindowSizeRight` |
| | Increase editor size right | `vim.increaseWindowSizeRight` |
| Control+W, O | Maximize editor | `workbench.action.maximizeEditor` |
| ~~Control+W, \|~~ | Maximize editor horizontally | `vim.maximizeWindowWidth` |
| ~~Control+W, _~~ | Maximize editor vertically | `vim.maximizeWindowHeight` |
| | Toggle editor group sizes | `workbench.action.toggleEditorWidths` |
| Control+W, = | Reset editor sizes | `workbench.action.evenEditorWidths` |

### Vim commands
| Default Key Binding | Description | Command |
| --- | --- | --- |
| Escape | Used to send `<ESC>` to Vim | `vim.esc` |

### Additional Commands

| Default Key Binding | Description | Command |
| --- | --- | ---
| n/a | Enable KeyDisplayer | `keyDisplayer.enable` |
| n/a | Disable KeyDisplayer | `keyDisplayer.disable` |

### Ex Commands

Commands that start with a `:` (colon) will be interpreted as vim ex commands. These can also carry arguments.

Examples:

```
  {"key": "kk", "command": ":split", "when": "editorTextFocus"},
  {"key": "<C-D>", "command": ":d 2", "when": "insertMode"}
```

### Leader Key

A leader key can be specified via the following configuration setting:

```
{ "vim.leader": "<space>" }
```
> NOTE: This setting is in `configuration.json`, not `keybindings.json`

Alternatively, the leader key can be specified via an `Ex` command:
```
:nmap <space> <Leader>
```

Once the leader key is defined, it may be used in both `keybindings.json` and via VimL map commands:

```
[
  { "key": "<Leader>p", "command": "workbench.action.quickOpen", "when": "editorTextFocus && normalMode" }
]
```

or, alternatively, in VimL:

```
:nnoremap <Leader>p <C-S-P>
```
