---
id: command-line
title: Command Line
sidebar_label: Command Line
---

# Command Line Interface

## Getting the oni2 Executable

## Windows 

The `oni2` command line executable is added to the user's `PATH` on Windows
during the Installation process, if the option is selected during installation.

## macOS

On macOS, `oni2` can be added manually with the "Add to System PATH" command in the
command pallette, by pressing `Cmd-Shift-P` and searching for `System`.

After selecting the option and giving admin permissions, the `oni2` executable
should be accessible.

> __NOTE:__ Admin access is required to add a symlink in `/usr/local/bin`. This
> should already be in your default path, but if it is not it can be added to
> your `.bashrc`/`.zshrc`, depending on your shell.

## Linux

On Linux, you should add an alias to `oni2`:

```sh
# Linux AppImage
alias oni2="${HOME}/path/to/Onivim2-x86_64.AppImage"

# Linux tar.gz
alias oni2="${HOME}/path/to/oni2/AppRun"
```

This should be added to one of the shell files that is loaded on shell start up, such as
the `.bashrc` or equivalent for other shells.

> __NOTE:__ It is currently preferable to alias `oni2`, rather than adding a symlink to it.

## Getting help

Launching `oni2` with the `--help` flag should give a brief outline on all command line
flags.

## General Usage

Passing `oni2` a file will open that file in Oni2, and set the open folder in Oni2 to the folder
that file is in. That is, `oni2 ~/my_project/docs/cli.md` will open `cli.md` in Oni2, and set the folder
to `~/my_project/docs/`. There is also a Zen mode configuration option around this single file mode, which
is outlined over [here](./../configuration/settings.md). By default, when opening with 1 file, `oni2` will
enter Zen mode, which can be disabled from the command pallette.

You can also pass over multiple files to open at once, and the first folder will be used as the current
working directory (unless over-ridden with the `--working-directory` flag.)

Launching `oni2` with a folder will open that folder. That is, `oni2 ~/git` will open `~/git` in Oni2
(for the file explorer, quick open and more). Launching without a folder will open Oni2 in the current
folder. The current folder can be changed once inside Oni2 by using the normal vim `:cd` command.

You can set Oni2 as the [default text editor for git](https://www.git-scm.com/book/en/v2/Customizing-Git-Git-Configuration#_code_core_editor_code) by running: 

```bash
git config --global core.editor "oni2 --nofork --silent"
```

## Extension Management

A more in detail explanation of the VSCode extension management can be found
[here](./../configuration/extensions.md).

By default, user extension are loaded from the following paths:

- Windows: `%LOCALAPPDATA%/Oni2/extensions`
- OSX & Linux: `~/.config/oni2/extensions`

This can be overridden via the `--extensions-dir`, like:

```
oni2 --extensions-dir /some/path/with/extensions
```

### List Extensions

Extensions can be listed with:
```
oni2 --list-extensions
```

### Install an Extension

You can manually install a Visual Studio Code extension packaged in a `.vsix` file:

```
oni2 --install-extension myextension.vsix
```

## Logging

The `-f` argument can be used to keep Onivim 2 attached to the terminal,
and provide logging output.

There are a also few options that can be specified using either environment variables or command line arguments:
- `ONI2_DEBUG` or `--debug` (e.g., `ONI2_DEBUG=1 oni2 -f` or `oni2 -f --debug`) - enable debug logging. This is very verbose but is helpful when logging issues!
- `--trace` - enable trace logging. This is extremely verbose!
- `--quiet` - print only error log messages. This can be sueful to quickly see if an extension failed to load, for example.
- `ONI2_LOG_FILE` or `--log-file` (e.g., `ONI2_LOG_FILE='oni.log' oni2` or `oni2 --log-file oni.log`) - enable logging to a file.
- `ONI2_LOG_FILTER` or `--log-filter` (e.g.. `ONI2_LOG_FILTER=Oni2.* oni2 -f` or `oni2 -f --log-filter "Oni2.*"`) - filter log messages using a comma-separated list of glob patterns matched against each message's namespace. Prefix a pattern with `-` to exclude rather than include matches. E.g. `ONI2_LOG_FILTER="Oni2.*, -*Ext*"` will include everything that matches `Oni2.*`, but exclude messages that also match `*Ext*`.

> __NOTE:__ Enabling debug logging will impact performance.

## Health check

There is a health-check utility bundled with Onivim 2, to verify the install
state. This can be run via:

```
oni2 -f --checkhealth
```

## Miscellaneous

- `-force-device-scale-factor` overrides the current scaling.

> Example: `oni2 --force-device-scale-factor 2`
