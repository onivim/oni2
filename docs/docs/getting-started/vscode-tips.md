### Tips for users coming from Visual Studio Code

If you come from Visual Studio Code, Onivim's appearance will hopefully have you
feeling right at home. It's behaviour might be a bit unfamiliar though, so here's
a few tips tips to help you along:

### How do I change the working directory?

In Normal mode, type `:cd <dir>`.

### How do I create a new file?

In Normal mode, type `:e <filename>`.

Note that this will eventually be possible to accomplish interactively using the
file explorer with either the keyboard or mouse.

### How do I create a new directory?

In Normal mode, type `:!mkdir <dir>`.

`:!` can be used to execute arbitrary commands, but has very limited feedback.

Note that this is a temporary stopgap. It will eventually be possible to
accomplish this interactively in the file explorer with either keyboard or mouse.

### How do I save a file?

In Normal mode, type `:w` to save or `:w <filename>` to save to a specific file.

### How do I close a file?

In Normal mode, type `:q`.

Note: If closing the last open file, this will also close the editor.

### How do I search the current file?

In Normal mode, type `/` to search forwards from the cursor, or `?` to search
backwards, followed by the pattern to search for.

Vim uses a peculiar regex-like syntax for patterns. See
[the documentation](http://vimdoc.sourceforge.net/htmldoc/pattern.html) for
details.

### How do I clear the search highlights?

In Normal mode, type `:nohlsearch`, or the short form `:noh`.
