---
id: snippets
title: Snippets in Onivim 2
sidebar_label: Snippets
---

Snippets are text templates that simplify entering blocks of source code. Snippets are especially useful for code blocks that are repeated frequently with minor variations.

![snippet-demo](https://user-images.githubusercontent.com/13532591/109021934-3276e700-7670-11eb-84d3-3637dc290016.gif)

In insert mode, as you type, Onivim shows available snippets alongside completion items.

The snippet syntax follows the [Code Snippet Syntax](https://code.visualstudio.com/docs/editor/userdefinedsnippets#_snippet-syntax) (which is a subset of the [TextMate Snippet Syntax](https://manual.macromates.com/en/snippets)). In addition, Onivim supports many of the same snippet configuration options as Visual Studio code, such as `"editor.snippetSuggestions"`.

## Using snippets

When a snippet is selected in the completion pop-up, pressing <kbd>Tab</kbd> will expand the snippet.

While a snippet is active, the mode will show as `Snippet` in the bottom-right, on the status bar. Pressing <kbd>Tab</kbd> while a snippet is active will move the cursor to the next placeholder, while pressing <kbd>Shift</kbd>+<kbd>Tab</kbd> will move the cursor to the previous placeholder. 

Once the cursor has been moved to the final placeholder, snippet mode will deactivate, and the editor will be returned to insert mode..

## Installing snippets

Onivim 2 comes bundled with snippets for several languages, including JavaScript, TypeScript, Reason, and others.

Extensions from [open-vsx](https://open-vsx.org/) can be installed to provide additional snippets:

![snippet-installation](https://user-images.githubusercontent.com/13532591/109021939-33a81400-7670-11eb-97d3-ebbcfa18c3cc.gif)

You can explore extensions that provide snippets by using the `@category:"snippets"` search filter in the extensions pane.

## Snippet syntax

Onivim's snippet syntax follows the same [snippet syntax as Code](https://code.visualstudio.com/docs/editor/userdefinedsnippets), including:

- __Tabstops:__ `$1`, `$2`, ... `$n` designate cursor positions within the snippet. `$0` is a special tabstop, designating the final cursor position. If there are multiple instances of the same tabstop, they will be synchronized and updated together.

- __Placeholders:__ Default values can be specified for tabstops, like `${1:default}`.

- __Variables:__ `$name` specifies a variable that can be inserted, such as `$CURRENT_YEAR`.

## Customizing snippets

![user-snippets](https://user-images.githubusercontent.com/13532591/109868107-1c42cb00-7c1c-11eb-9abf-49f762e57d33.gif)

Snippets may be customized globally or per-filetype - these are stored in the user `snippets` folder:
- If the snippet file ends with extension `.json`, the filename is the relevant filetype - for example, `bat.json` provides snippets for the `bat` filetype.
- If the snippet file ends with extension `.code-snippets`, the snippets file applies globally, to all filetypes.

To configure your snippets:
- Open the command palette (<kbd>Control</kbd>+<kbd>Shift</kbd>+<kbd>P</kbd> / <kbd>Command</kbd>+<kbd>Shift</kbd>+<kbd>P</kbd>)
- Select "Configure user snippets"
- Choose the filetype to modify
- Press <kbd>Enter</kbd> to open the relevant snippet file
- Modify and save

## Binding a snippet to a key

Snippets can also be bound to keys, using the `"editor.action.insertSnippet"` command, for example:

![snippet-key-binding](https://user-images.githubusercontent.com/13532591/109041574-082f2480-7684-11eb-9ee4-bc7bcbc89a2c.gif)
