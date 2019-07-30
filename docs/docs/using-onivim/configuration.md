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
