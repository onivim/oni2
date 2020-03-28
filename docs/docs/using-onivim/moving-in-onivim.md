---
id: moving-in-onivim
title: Moving Around
sidebar_label: Moving Around
---

## Basic Cursor Movement

In _normal mode_, the cursor can be moved with the following keys:

- <kbd>h</kbd> - move left
- <kbd>j</kbd> - move down
- <kbd>k</kbd> - move right
- <kbd>l</kbd> - move down

These keys can be prefixed with a number: <kbd>1</kbd><kbd>0</kbd><kbd>j</kbd> would jump down 10 lines.

You can jump to around a buffer with the <kbd>G</kbd> family of commands:

- <kbd>g</kbd><kbd>g</kbd> - Jump to the top of a buffer
- <kbd>G</kbd> - Jump to the bottom of a buffer
- <kbd>`0-9*`</kbd><kbd>G</kbd> - Jump to line number `digit`. For example, <kbd>5</kbd><kbd>0</kbd><kbd>G</kbd> jumps to line 50.

<center>
	<figure>
		<video autoplay loop muted playsinline>
			<source src="/vid/demo-gg.mp4" type="video/mp4">
			<source src="/vid/demo-gg.webm" type="video/webm">
		</video>
		<figcaption>
			<i>
				Moving around with <kbd>G</kbd> and <kbd>g</kbd><kbd>g</kbd>
			</i>
			</figcaption>
	</figure>
</center>

It also helps to be able to move around within a line:

- <kbd>0</kbd> (or <kbd>Home</kbd>) - Move to the first character in a line.
- <kbd>^</kbd> - Move to the first non-blank character of a line.
- <kbd>$</kbd> (or <kbd>End</kbd>) - Move to the last character of a line.

## Searching

Often, we have an idea of what we're looking for - maybe a keyword or part of a name. 

The search motion - <kbd>//</kbd> - can be used for moving the cursor to a particular search string:

- <kbd>//</kbd>`search-string`<kbd>Return</kbd> will search forward, moving the cursor the next instance of `search-string` found in the buffer
- <kbd>?</kbd>`search-string`<kbd>Return</kbd> will search backward, moving the cursor to the previous instance of `search-string`
- <kbd>N</kbd> will move the cursor to the next instance of the last used `search-string`
- <kbd>n</kbd> will move the cursor to the previous instance of the last used `search-string`

- <kbd>*</kbd> will search for the next instance of the identifier under the cursor.
- <kbd>%</kbd> will move the cursor to a matching bracket.

## Sneak Mode

Vim's model of modal editing, which Onivim is based off of, was really designed for terminal user interfaces. 

However, Onivim provides a graphical user interface - and we want the entirety of our user interface to be keyboard-accessible. If you can't do something via the keyboard, it's a bug!

'Sneak mode' provides a bridge for features that are very visual. By pressing <kbd>Control</kbd>+<kbd>g</kbd>, you can enter sneak mode:

When sneak mode is active, all visual elements will be tagged with an identifier - typing that identifier will perform the same action as clicking with the mouse.


