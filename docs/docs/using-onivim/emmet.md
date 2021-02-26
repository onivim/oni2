---
id: emmet
title: Emmet in Onivim 2
sidebar_label: Emmet
---

Onivim 2 supports basic [Emmet expansions](https://docs.emmet.io/abbreviations/) out-of-the-box, no extensions needed:

![emmet-demo](https://user-images.githubusercontent.com/13532591/109021943-34d94100-7670-11eb-88ba-bb8f96085d30.gif)

This is provided by a bundled [Emmet extension](https://github.com/onivim/oni2/tree/master/extensions/emmet).

## Supported files

The following file types are supported:
- `html`
- `css`
- `less`
- `sass`

## Usage

As you type, if an Emmet expansion is available, the expansion will be shown in the completion pop-up alongside other completion items. 

Pressing <kbd>Tab</kbd> will expand the abbreviation.

Once expanded, the text behaves like a [snippet](./snippet) - and <kbd>Tab</kbd> and <kbd>Shift</kbd>+<kbd>Tab</kbd> can be used to navigate the placeholders.

## Recommended configuration

The following [configuration](../configuration/settings.md) is recommended for using Emmet with Onivim 2:

```
"emmet.showSuggestionsAsSnippets": true,
"editor.snippetSuggestions": "top"
```

This ensures that Emmet expansions take priority over other completion items.
