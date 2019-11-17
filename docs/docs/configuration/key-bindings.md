p---
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
{ "key": "<C-P">, command: "quickOpenFiles", "when": "editorTextFocus" }
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

There are a set of default rules provided by Onivim, but the customized rules are appended to the bottom - thus, user key bindings are esxecuted first.

### `key` format

TODO

### `command` arguments

### `when` clause contexts

Onivim 2 gives you control over when key bindings are enabled through the `when` clause. 

#### Conditional operators

For conditional expressions, you can use the following conditional operators:

| Operator | Symbol | Example |
| --- | --- | --- |
| Or | <code>&#124;&#124;</code> | <code>menuFocus &#124;&#124; textInputFocus</code> |
| And | `&&` | `insertMode && suggestWidgetVisible` |

Expressions may be grouped with `()`, for example:
```
(menuFocus && !insertMode) || suggestWidgetVisible
```

#### Contexts

Common contexts with VSCode:

| Context Name | True When | 
| --- | --- |
| `editorFocus` | An editor has focus |
| `textInputFocus` | A text input area has focus |
| `suggestWidgetVisible` | The suggest widget (auto-completion) is visible |

Onivim-specific contexts:

| Context Name | True When | 
| --- | --- |
| `insertMode` |  The active editor is in `insert` mode |
| `commandLineFocus` | The Vim commandline is open |
| `menuFocus` | A pop-up menu has focus |


## Default Keyboard Shortcuts

TODO


