---
id: modal-editing-101
title: Modal Editing 101
sidebar_label: Modal Editing 101
---

## Modal Editing

Onivim, like Vi and Vim, is a __modal editor__ - meaning it operates in different _modes_.

The default mode for Onivim, like Vi and Vim, is _normal mode_. Normal mode is a command mode; characters you type execute commands (like moving the cursor, deleting text, etc).
This is much different from _modeless_ editors; where typing always inserts text! The advantage of having _modes_ is that you can navigate through code, manipulate text swiftly,
and even manage selections without your hands leaving the keyboard.

### Normal Mode

'Normal' mode seems counter-intuitive - coming from other _modeless_ editors, the 'normal' mode seems like it should be inserting characters! However, when working with text,
the majority of our time is spent navigating, manipulating, and editing - rather than inserting. For this reason, the 'normal' mode is the default mode.

In normal mode, you can move the cursor around:

- `h` - move the cursor left
- `j` - move the cursor down
- `l` - move the cursor right
- `k` - move the cursor up

You can also prefix these characters with a number, for example:
- `5j` moves the cursor 5 lines down
- `10l` moves the cursor 10 lines right

### Insert Mode

Even though we spend a lot time in _normal mode_, we need to be able to type and insert text, too. To enter _insert mode_, you can press `i` from _normal mode_.

The cursor will switch to a caret, and you'll see an indication in the status bar that we are now in insert mode.

Once you have finished typing, you can press `Escape`, `Control+c`, or `Control+[` to return to _normal mode_ - pick whichever one is most comfortable for you.

### Ex Commands

Onivim also exposes a vast number of vim's ex commands, which are key sequences starting with `:`. Some useful ones to get started with are:

- `:cd <directory>` to change the working directory
- `:e <filename>` to open a file for editing, or create a new file if it doesn't exist
- `:w` to save the current file
- `:w <filename>` to save the current file with a new name
- `:q` to close the current file, and the editor if it the last open file
- `:wq` as a shorthand to save and close

### Next Steps

Play around with switching between insert mode and normal mode, and when you're ready, let's dive-in to some more advanced editing:
- [Moving in Onivim](../using-onivim/moving-in-onivim)
- [Editing & Deleting Text](../using-onivim/editing-and-deleting)
- [Visual Mode (Selection)](../using-onivim/visual-mode)
- [Working with Files](../using-onivim/editing-and-deleting)
- [Configuration](../using-onivim/configuration)





