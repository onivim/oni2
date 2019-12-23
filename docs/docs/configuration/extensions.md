---
id: extensions
title: VSCode Extensions
sidebar_label: VSCode Extensions
---

Onivim is capable of loading VSCode extensions. The current capabilities of the
extension host can be found [here](https://github.com/onivim/oni2/issues/1058).

## Installing extensions

Onivim has two ways of installing VSCode extensions currently, though in the future this
will be expanded to include an in-editor experience.

Extensions by default are installed in the following folders:

 - `~/.config/oni2/extensions` - Linux/macOS
 - `%LOCALAPPDATA%/Oni2/extensions` - Windows

This can be overridden with the `--extensions-dir` command line flag however.

### Installing `vsix` files

The Onivim command line executable has a `--install-extension` flag, that can be given a
path to a `.vsix` file like so:

```sh
oni2 --install-extension /path/to/some-extension.vsix
```

> __NOTE:__ Currently, `oni2` is only added to the user's `PATH` on Windows. Find instructions on adding it to your `PATH` on macOS and Linux over [here](./../using-onivim/command-line.md).

### Installing manually

If a `.vsix` file can not be found, the alternative way of installing a VSCode extension
is to download the extension manually into the extension folder.

```sh
cd $ONI2_CONFIG_DIR/extensions
git clone https://github.com/thomaspink/vscode-github-theme.git
```

## Listing extensions

The currently installed extensions are listed in the editor, in the extensions panel.
Alternatively, this can be achieved with the `--list-extensions` command line flag.
