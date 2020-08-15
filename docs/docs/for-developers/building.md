---
id: building
title: Building from Source
sidebar_label: Building from Source
---

# Building the Editor

## Prerequisites

- Install [Git](https://git-scm.com/)
- Install [Node](https://nodejs.org/en)
- Install [Esy](https://esy.sh) (__0.6.2__ or above is required, but the latest version is recommened: `npm install -g esy@latest`)
- __Windows-only__: Run `npm install -g windows-build-tools` (this installs some build tools that aren't included by default on Windows)
- Install any other system packages required by Oni2 dependencies, as outlined below.

## Dependencies

All platforms require `git`, `node` and `esy`, and the ones outlined in the `revery` docs:
https://github.com/revery-ui/revery/wiki/Building-&-Installing.

### Windows

No additional requirements.

### macOS

Requires `libtool` and `gettext` from homebrew: `brew install libtool gettext`.

### Linux

Some Linux distributions may need other packages:

 - Ubuntu : `libacl1-dev`, `libncurses-dev` for `libvim`.
 - Fedora/CentOS : `libXt-devel`, `libSM-devel`, `libICE-devel` for `libvim`

## Build and Run

### Build the front-end

> __NOTE:__ On Windows, you must __build from a shell running as administrator__. This is a requirement of esy because creating symlinks requires administrator permissions. More info at [esy/esy#389](https://github.com/esy/esy/issues/389).

> __NOTE:__ On macOS, if you receive an `error: Too many open files`, you can run `ulimit -Sn 4096` to increase the file limit. More info at [esy/esy#1057](https://github.com/esy/esy/issues/1057)

```sh
git clone https://github.com/onivim/oni2
cd oni2
esy install
esy bootstrap
esy build
```

### Install node dependencies

```sh
npm install -g node-gyp
node install-node-deps.js
```

> __NOTE:__ The non-standard `node install-node-deps.js` step instead of `npm install` is necessary because the script picks up our _vendored_ node binary.

### Run health-check

> The `--checkhealth` argument validates all dependencies are available and working.

- `esy run -f --checkhealth`


### Run Onivim 2

- `esy run`

### Tests

- `esy '@test' install`
- `esy '@test' build`
- `esy '@test' run`

### Benchmarks

- `esy '@bench' install`
- `esy '@bench' build`
- `esy '@bench' run`

## Release 

To create a release build, run:

- `esy x Oni2 -f --checkhealth`
- `esy create-release`

This will create a `_release` folder at the root with the application bundle inside.

### Windows

### OSX

Once you have a release build created, you can install Oni2 in your `Applications` folder.

Run the following from the `oni2` directory:

- `cp -R _release/Onivim2.app $USER/Applications`

If you want to open the editor from terminal with an `oni2` command, you can add Oni2 to the system PATH using a command
item within the app:

- Open Oni2 from launchpad
- Open command palette with `Cmd + Shift + P`
- Execute: `System: Add Oni2 to System PATH`

## Extension Integration

If you want to develop, or debug, an extension integration, the following tips may help:

### Testing with oni-dev-extenion

There is a development extension in `src/development_extensions/oni-dev-extension` which can be used to implement dummy functionality that is often easier to test and integrate with than real extensions.

#### Resources
- [VS Code API reference](https://code.visualstudio.com/api/references/vscode-api)

### Intrumenting extensions

To add logging, use `console.error` - messages on `stderr` will be shown in Onivim's log. (Make sure to turn debug logging on, via `ONI2_DEBUG=1` environment variable or the `--debug` command-line arg).

Both the oni-dev-extension and any other extension can be instrumented, as they're usually included in non-minified form.

### Extension host

If there's a problem in-between Oni2 and the extension, it can be helpful to build and instrument your own copy of the extension host.

#### Building

- Navigate to your home directory (ie, `cd ~` on Linux / OSX, or `cd /` on Windows)
- `git clone https://github.com/onivim/vscode-exthost`
- `yarn install`
- `yarn compile`

You can use the `yarn watch` command too - this is useful for iterating quickly!

#### Testing

You can use the `ONI2_EXTHOST` environment variable to override the default extension host with your local extension host:
- `ONI2_EXTHOST=/Users/<your-username>/vscode-exthost esy run -f --debug``

For example, adding the logging here (the [`$executeContributedCommand`](https://github.com/onivim/vscode-exthost/blob/a25f426a04fe427beab7465be660f89a794605b5/src/vs/workbench/api/node/extHostCommands.ts#L165) proxy method)

![image](https://user-images.githubusercontent.com/13532591/72770589-3013a500-3bb3-11ea-9c24-805bfe1cb7d1.png)

Results in this debug logging:

![image](https://user-images.githubusercontent.com/13532591/72770839-ed9e9800-3bb3-11ea-9cb9-317223fb2dbb.png)

#### Protocol

The extension host protocol is defined in [extHost.protocol.ts](https://github.com/onivim/vscode-exthost/blob/master/src/vs/workbench/api/node/extHost.protocol.ts). Interfaces prefixed with `MainThread` refer to messages sent from the extension host to the "main thread", which in our case is Oni2. While interfaces prefixed with `ExtHost` refer to messages sent _to_ the extension host.

# Building the Documentation Website

From the `oni2` directory:

- `cd docs/website`
- `npm install`
- `npm start`
