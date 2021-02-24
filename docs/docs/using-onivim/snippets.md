---
id: snippets
title: Snippets in Onivim 2
sidebar_label: Snippets
---

Snippets are text templates that simplify entering code patterns,
and are especially useful for code blocks that are used repeatedly, with minor variations.

![snippet-demo](https://user-images.githubusercontent.com/13532591/109021934-3276e700-7670-11eb-84d3-3637dc290016.gif)

Onivim shows available snippets along with completion items as you type in insert mode.

The snippet syntax follows the [Code Snippet Syntax](https://code.visualstudio.com/docs/editor/userdefinedsnippets#_snippet-syntax) (which is a subset of the [TextMate Snippet Syntax](https://manual.macromates.com/en/snippets)). In addition, Onivim supports many of the same configuration options as Visual Studio code.

## Using snippets

When a snippet is available in the completion, pressing <kbd>Tab</kbd> will expand the snippet.

While a snippet is active, the mode will show as `Snippet` in the statusbar. Pressing <kbd>Tab</kbd> while a snippet is active will move the cursor to the next placeholder, while pressing <kbd>Shift</kbd>+<kbd>Tab</kbd> will move to the previous placeholder.

Once the cursor has been moved to the final placeholder, snippet mode will deactivate.

## Installing snippets

Onivim 2 comes with bundled snippets for several languages, including JavaScript, TypeScript, Reason, and others.

Extensions from [open-vsx](https://open-vsx.org/) can provide additional snippets:

![snippet-installation](https://user-images.githubusercontent.com/13532591/109021939-33a81400-7670-11eb-97d3-ebbcfa18c3cc.gif)

You can explore extensions that provide snippets by using the `@category:"snippets"` search filter.

## Snippet syntax

Onivim's snippet syntax follows the same [snippet syntax as Code](https://code.visualstudio.com/docs/editor/userdefinedsnippets), including:

- __Tabstops:__ `$1`, `$2`, ... `$n` designate cursor positions within the snippet. `$0` is a special tabstop, designating the final cursor position. If there are multiple instances of the same tabstop, they will be synchronized and updated together.

- __Placeholders:__ Default values can be specified for tabstops, like `${1:default}`.

- __Variables:__ `$name` specifies a variable that can be inserted, such as `$CURRENT_YEAR`.

## Customizing user snippets

![snippet-user-config](https://user-images.githubusercontent.com/13532591/109041574-082f2480-7684-11eb-9ee4-bc7bcbc89a2c.gif)

Snippets can be customized globally or per-filetype - these are stored in the user `snippets` folder:
- If the filetype ends in `.json`, the filename is the relevant filetype - for example, `bat.json` provides snippets for .bat files.
- If the filetype ends in `.code-snippets`, the snippets file applies globally.

To configure your snippets:
- Open the command palette (<kbd>Control</kbd>+<kbd>Shift</kbd>+<kbd>P</kbd> / <kbd>Command</kbd>+<kbd>Shift</kbd>+<kbd>P</kbd>)
- Select "Configure user snippets"
- Choose the filetype to modify
- Press <kbd>Enter</kbd> to open the relevant snippet file
- Modify and save

## Binding a snippet to a key

Snippets can also be bound to keys, using the `"editor.action.insertSnippet"` command, for example:

![snippet-key-binding](https://user-images.githubusercontent.com/13532591/109041574-082f2480-7684-11eb-9ee4-bc7bcbc89a2c.gif)
