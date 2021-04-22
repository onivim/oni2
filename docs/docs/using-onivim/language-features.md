---
id: language-features
title: Language Features
sidebar_label: Language Features
---

## Code Completion

![image](https://user-images.githubusercontent.com/13532591/88990707-8dd82680-d293-11ea-8dae-eac852a77f2b.png)

Onivim will automatically show code completion, when available. Code completion is only available in insert mode.

Completion items are fuzzy-matched, so typing `sdn` will select `stdin` in the example above.

__Keybindings__

- <kbd>Control</kbd>+<kbd>n</kbd> or <kbd>Down</kbd> - switch to next completion item.
- <kbd>Control</kbd>+<kbd>p</kbd> or <kbd>Up</kbd> - switch to previous completion item.
- <kbd>Tab</kbd> - complete the selected item.

## Hover

![hover](https://user-images.githubusercontent.com/13532591/88988567-19e74f80-d28e-11ea-98d3-25391c9790c1.png)

Hover shows detailed information about a variable or identifier, as well as any diagnostic errors.

__Keybindings__

- <kbd>g</kbd><kbd>h</kbd> _normal mode_ - open hover
- <kbd>Escape</kbd> - close open hover

## Signature Help

![signature-help](https://user-images.githubusercontent.com/13532591/88990342-91b77900-d292-11ea-91ef-5d856d816adc.png)

When available, signature help will be automatically opened when typing a trigger character (defined by the extension) - for example, `(`.

## Go-to Definition

If a definition is available, the identifier will be underlined:

![go-to-definition](https://user-images.githubusercontent.com/13532591/88990595-3c2f9c00-d293-11ea-9642-c366efb40b69.png)

__Keybindings__

- <kbd>g</kbd><kbd>d</kbd> will open the definition location.

## Formatting

The `=` operator can be used to format, using the best available formatter.

For example:

- <kbd>g</kbd><kbd>g</kbd><kbd>=</kbd><kbd>G</kbd> - format entire document

If no formatter is available, Onivim will fall-back to the language-defined indentation rules.

## Auto-Closing Pairs

Onivim will automatically insert closing pairs, based on the language configuration settings.

For example, if the language defines `(` and `)` as closing pairs, typing `(` will insert `(|)`.

Typing the closing pair will result in the cursor 'passing through'. Pressing <kbd>Backspace</kbd> in a closing pair will remove both the opening and closing pair.

## Symbol Outline

If the language extension supports it, `gO` can be used to navigate to the symbol outline.

In addition, `gs` can be used in normal to go-to a specific symbol in the file:!

[goto-symbol](https://user-images.githubusercontent.com/13532591/114098552-57b56280-9876-11eb-8298-d9f764efd48f.gif)
