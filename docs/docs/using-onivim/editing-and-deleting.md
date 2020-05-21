---
id: editing-and-deleting
title: Editing and Deleting Text
sidebar_label: Editing and Deleting Text
---

## Insert Mode

Insert mode is the mode you usually think about in a text editor - when you press a key, the corresponding character is inserted.

To enter `insert` mode from `normal` mode:

- <kbd>i</kbd> - enter `insert` mode before the cursor position, on the same line.
- <kbd>I</kbd> - enter `insert` mode at the beginning of the current line.
- <kbd>a</kbd> - enter `insert` mode after the cursor position, on the same line.
- <kbd>A</kbd> - enter `insert` mode at the end of the current line.
- <kbd>o</kbd> - create a line after the current line, and enter `insert` mode.
- <kbd>O</kbd> - create a line before the current line, and enter `insert` mode.

To leave `insert` mode, and go back to `normal` mode, you can use the following keys:

- <kbd>Escape</kbd>
- <kbd>Control</kbd>+<kbd>c</kbd>
- <kbd>Control</kbd>+<kbd>[</kbd>

## Operators

Besides insert mode, there are other ways to manipulate text in `normal` mode - that's where operators come in. Vim and the 'vi' family of editors,
like Onivim, provide a sort of text-editing-language, and operators are an important piece of this language.

### Basic Operators

- <kbd>d</kbd>_motion_ - __delete__ the text described by _motion_.
- <kbd>c</kbd>_motion_ - __change__ the text described by _motion_.

A motion is a cursor movement, as described in [Moving Around](./moving-in-onivim.md).

Examples:

- <kbd>c</kbd><kbd>$</kbd> - change the text from the cursor position to the end-of-theline.
- <kbd>d</kbd><kbd>d</kbd> - delete the current line.
- <kbd>d</kbd><kbd>10</kbd><kbd>j</kbd> - delete the 10 lines below the cursor.
- <kbd>d</kbd><kbd>G</kbd> - delete to the end of the file.

### Other operators

- <kbd>y</kbd>_motion_ - _yank_ text to a specified register.
- <kbd>gu</kbd>_motion_ - convert text described by _motion_ to lowercase.
- <kbd>gU</kbd>_motion_ - convert text described by _motion_ to uppercase.
- <kbd>>></kbd>_motion_ - shift lines described by _motion_ to the right.
- <kbd><<</kbd>_motion_ - shift lines described by _motion_ to the left.

## Repeat

The <kbd>.</kbd> "dot operator" is a very useful command - it repeats the last command.

For example, if <kbd>d</kbd><kbd>d</kbd> was used to delete a line,
pressing the <kbd>.</kbd> key would repeat the command, and delete another line.

## Undo / Redo

It can be scary to experiment with the operators and motions, for fear of losing your work!

However, you can always undo your change:

- In `insert` mode, you can use <kbd>Command</kbd>+<kbd>z</kbd> (<kbd>Control</kbd>+<kbd>z</kbd> on other platforms)
- In `normal` mode, <kbd>u</kbd> will undo the last change, while <kbd>Control</kbd>+<kbd>r</kbd> will redo it.

## Further Reading

We've only scratched the surface of motions and operators available here - 
checkout the [Vim documentation on motions and operators](http://vimdoc.sourceforge.net/htmldoc/motion.html)
for more.
