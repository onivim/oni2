---
id: c
title: C / C++
sidebar_label: C / C++
---

## ctags

Onivim supports ctags via the [`exuberant-ctags`](https://open-vsx.org/extension/chriswheeldon/exuberant-ctags).

![ctags](https://user-images.githubusercontent.com/13532591/88988212-11424980-d28d-11ea-9dde-8fe2d7ee7407.png)

The following features are supported:
- Code completion
- Go-to definition
- Symbol search
- Commands to regenerate the ctags file

Install via the command-line:

- `oni2 --install-extension chriswheeldon.exuberant-ctags`

Once installed, there will be a status bar entry showing the state of the CTags index:

![image](https://user-images.githubusercontent.com/13532591/88988236-24551980-d28d-11ea-9f1a-ff7ace3d6c79.png)

## clangd

Onivim supports the [`clangd`](https://open-vsx.org/extension/llvm-vs-code-extensions/vscode-clangd) plugin:

![clangd-hover](https://user-images.githubusercontent.com/13532591/88988567-19e74f80-d28e-11ea-98d3-25391c9790c1.png)

The following features are supported:
- Code completion
- Go-to definition
- Hover
- Compile errors and warnings
- Code formatting

Install via the command-line:

- `oni2 --install-extension llvm-vs-code-extensions.vscode-clangd`

For best results, you'll need to generate a `compile_commands.json` file.

The first time you run, you'll be prompted to install `clangd`:
![clangd-dialog](https://user-images.githubusercontent.com/13532591/88988750-9bd77880-d28e-11ea-9fc9-1c3b44c228ab.png)

This will set the `clangd.path` configuration option.
