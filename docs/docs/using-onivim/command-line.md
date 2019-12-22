---
id: command-line
title: Command Line
sidebar_label: Command Line
---

## Command Line Interface

The `oni2` command line executable is currently only added to the user's `PATH` on
Windows. However, it can be added manually by updating the `PATH` environment variable
to point to the folder where the `oni2` executable lives, or adding an alias to `oni2`:

```sh
# Linux
alias oni2="${HOME}/path/to/Onivim2-x86_64.AppImage"

# macOS
alias oni2='/Applications/Onivim2.app/Contents/MacOS/Oni2'
```

This should be added to one of the shell files that is loaded on shell start up, such as
the `.bashrc` or equivalent for other shells.

### Getting help

Launching `oni2` with the `--help` flag should give a brief outline on all command line
flags.

### Extension Management

By default, user extension are loaded from the following paths:

- Windows: `%LOCALAPPDATA%/Oni2/extensions`
- OSX & Linux: `~/.config/oni2/extensions`

This can be overridden via the `--extensions-dir`, like:

```
oni2 --extensions-dir /some/path/with/extensions
```

#### List Extensions

Extensions can be listed with:
```
oni2 --list-extensions
```

#### Install an Extension

You can manually install a Visual Studio Code extension packaged in a `.vsix` file:

```
oni2 --install-extension myextension.vsix
```

### Logging

The `-f` argument can be used to keep Onivim 2 attached to the terminal,
and provide logging output.

There are a also few options that can be specified using either environment variables or command line arguments:
- `ONI2_DEBUG` or `--debug` (e.g., `ONI2_DEBUG=1 oni2 -f` or `oni2 -f --debug`) - enable debug logging. This is very verbose but is helpful when logging issues!
- `ONI2_LOG_FILE` or `--log-file` (e.g., `ONI2_LOG_FILE='oni.log' oni2` or `oni2 --log-file oni.log`) - enable logging to a file.
- `ONI2_LOG_FILTER` or `--log-filter` (e.g.. `ONI2_LOG_FILTER=Oni2.* oni2 -f` or `oni2 -f --log-filter "Oni2.*"`) - filter log messages using a comma-separated list of glob patterns matched against each message's namespace. Prefix a pattern with `-` to exclude rather than include matches. E.g. `ONI2_LOG_FILTER="Oni2.*, -*Ext*"` will include everything that matches `Oni2.*`, but exclude messages that also match `*Ext*`.

> __NOTE:__ Enabling debug logging will impact performance.

### Healthcheck

There is a health-check utility bundled with Onivim 2, to verify the install
state. This can be run via:

```
oni2 -f --checkhealth
```

### Miscellaneous

- `-force-device-scale-factor` overrides the current scaling. 

> Example: `oni2 --force-device-scale-factor 2`
