
/* `keyPressToCommand` converts a KeyPress into a Vim-style string command.
/*
/*  Returns `None` if the key press should not be handled by Vim.
/*  Otherwise, returns `Some(keyString)`
let keyPressToCommand: (~isTextInputActive: bool, EditorInput.KeyPress.t) => option(string);
