---
id: working-with-files
title: Working with Files
sidebar_label: Working with Files
---

## Navigating the File Explorer

To change the current working directory, which will also update the file explorer root, you can use the `:cd` command - for example: `:cd ~/my/project`.

## Opening files

### QuickOpen

Onivim 2 has a QuickOpen fuzzy-finder out-of-the-box, powered by [ripgrep](https://github.com/burntsushi/ripgrep).

To access the QuickOpen fuzzy-finder, you can use:
- __Windows / Linux:__ `Control+P`
- __OSX__ `Command+P`

### `:e` command

You can also use Vim's Ex-mode `edit` command to open a file - for example: `:e dit~/my/project/README.md`.

> __NOTE:__ `:edit` can be abbreviated as `:e`.

## Saving files

To save your changes, you can use the `:write` command.

> __NOTE:__ `:write` can be abbreviated as `:w`

If you want to save to another file, you can use the `:save` command, for example: `:sav ~/my/project/another-file.txt`.

## Navigating in opened windows

By pressing `<C+TAB>` you can access the currently opened windows. Similar to open all files you can navigate through files.
