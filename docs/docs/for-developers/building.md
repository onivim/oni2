---
id: building
title: Building from Source
sidebar_label: Building from Source
---

# Building the Editor

## Prerequisites

- Install [Git](https://git-scm.com/)
- Install [Node](https://nodejs.org/en)
- Install [Esy](https://esy.sh) (__0.5.6__ is required)
- __Windows-only__: Run `npm install -g windows-build-tools` (this installs some build tools that aren't included by default on Windows)
- [Check and install any system packages for Revery](https://github.com/revery-ui/revery/wiki/Building-&-Installing)

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

### Run

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
