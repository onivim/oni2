---
id: command-line
title: Command Line
sidebar_label: Command Line
---

## Command Line Interface

### Getting help

### Managing extensions

By default, user extension are loaded from the following paths:

- Windows: `%LOCALAPPDATA%/Oni2/extensions` 
- OSX & Linux: `~/.config/oni2/extensions`

This can be overridden via the `--extensions-dir`, like:

```
oni2 --extensions-dir /some/path/with/extensions
```

### Logging

The `-f` argument can be used to keep Onivim 2 attached to the terminal,
and provide logging output.

### Healthcheck

There is a health-check utility bundled with Onivim 2, to verify the install
state. This can be run via:

```
oni2 -f --checkhealth
```

### Miscellaneous

- `-force-device-scale-factor` overrides the current scaling. 

> Example: `oni2 --force-device-scale-factor 2`
