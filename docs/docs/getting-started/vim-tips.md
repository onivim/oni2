---
id: vim-tips
title: Tips for users coming from Vim
sidebar_label: Tips for Vim Users
---

### How do I get the tabs to behave as they do in Vim?

Add these settings to your configuration:

```
  "oni.layout.singleTabMode": true,
  "oni.layout.layoutTabPosition": "top",
```

### How do I replicate my .vimrc?

Using the
[`"experimental.viml"`](https://onivim.github.io/docs/configuration/settings#experimental)
setting you can already successfully run many VimL commands like keybindings.
But it is experimental for good reason. Many commands do not work, and there's
no overview of what does and does not work.

This setting will eventually be replaced with a mechanism that will support a
larger and more well-defined subset of VimL. See
[this issue](https://github.com/onivim/oni2/issues/150) for details.

### How do I bind to Vim motions and commands using Oni's native keybinding configuration?

Ex commands are supported by prefixing the command with `:` as you would when
typing it in Normal mode. For example:

```
  {"key": "kk", "command": ":split", "when": "editorTextFocus"},
  {"key": "<C-D>", "command": ":d 2", "when": "insertMode"}
```

There's currently no support for creating native keybindings for Vim motions.
For now, please use VimL keybindings with the
[`"experimental.viml"`](https://onivim.github.io/docs/configuration/settings#experimental)
setting.
