---
id: building
title: Building from Source
sidebar_label: Building from Source
---

# Building the Editor

## Prerequisites

- Install [Git](https://git-scm.com/)
- Install [Node](https://nodejs.org/en)
- Install [Esy](https://esy.sh) (__0.6.0__ is required)
- __Windows-only__: Run `npm install -g windows-build-tools` (this installs some build tools that aren't included by default on Windows)
- Install any other system packages required by Oni2 dependencies, as outlined below.

## Dependencies

### Windows, macOS

There should be no required system dependencies, outside of `git`, `node` and `esy` and the
ones outlined in the `revery` docs: https://github.com/revery-ui/revery/wiki/Building-&-Installing.

### Linux

Like the other platforms, `git`, `node` and `esy` are required, as well as any outlined in
the `revery` docs: https://github.com/revery-ui/revery/wiki/Building-&-Installing.

Some Linux distributions may need other packages:

 - Ubuntu : `libacl1-dev`, `libncurses-dev` for `libvim`.

## Build and Run

### Build the front-end

> __NOTE:__ On Windows, you must __build from a shell running as administrator__. This is a requirement of esy because creating symlinks requires administrator permissions (more info here: https://github.com/esy/esy/issues/389).

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

Once you have a release build created, you can create an `oni2` symlink to point to your development environment.

Run the following from the `oni2` directory:
- `./scripts/osx/create-symlink.sh`

# Building the Documentation Website

From the `oni2` directory:

- `cd docs/website`
- `npm install`
- `npm start`
