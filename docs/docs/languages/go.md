---
id: go
title: Go
sidebar_label: Go
---

Onivim supports the [vscode-go](https://github.com/golang/vscode-go) extension:

![go](https://user-images.githubusercontent.com/13532591/87736719-bf2dfe00-c78d-11ea-89ed-147eea8d0cae.png)

Several language integration features are supported, like:
- Code completion
- Go-to definition
- Formatting
- Signature help

## Setup

1) Install the 'go' extension via the extensions pane
2) Restart Onivim

## Recommended configuration

The `vscode-go` extension supports various toolchains, but we recommend using the `gopls` language server:

1) Run `go get golang.org/x/tools/gopls@latest`
2) Set `"go.useLanguageServer": true` in your configuration.

For more information, see the [`gopls` documentation](https://github.com/golang/vscode-go/blob/master/docs/gopls.md)
