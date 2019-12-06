# vscode-reason-language-server

Based on `vscode-reasonml`, but with a language server backend that's written entirely in reason & compiled natively.

![screenshot](https://github.com/jaredly/reason-language-server/raw/master/editor-extensions/vscode/screenshot.png)

## Features

- hover goodness
- autocomplete
- function signature help
- jump-to-definition & preview-definition
- "Find all references"
- document outline / symbol search
- code lens

  - Show a file's dependencies at the top
  - Show what values are used from an `open`
  - Per-value type codelens (off by default)

- highlight all usages of a variable
- rename a variable
- format selection
- format document
- auto-run bsb / dune on file change

## Configuration
all configuration is prefixed with `reason_language_server.`

- `.format_width` - defaults to 80
- `.per_value_codelens` - show the type of each toplevel value in a lens
- `.dependencies_codelens` - list a files dependencies at the top
- `.opens_codelens` - show what values are used from an `open`
- `.autoRebuild` — rebuild project on save (turned on by default)

## Debugging configuration
most useful if your developing the language server

- `.location` - provide a custom binary location for the langauge server
- `.refmt` - provide a custom location for refmt
- `.lispRefmt` - provide a custom location for reason-lisp's refmt
- `.reloadOnChange` - reload the server when the binary is updated
- `.show_debug_errors` - pipe the server's stderr into vscode's output pane

## Copyright & License

Copyright © 2018 Jared Forsyth and contributors.

Distributed under the MIT License (see [LICENSE](https://github.com/jaredly/reason-language-server/blob/master/./LICENSE)).
