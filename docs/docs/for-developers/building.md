---
id: building
title: Building from Source
sidebar_label: Building from Source
---

## Prerequisites

- Install [Git](https://git-scm.com/)
- Install [Node](https://nodejs.org/en)
- Install [Esy](https://esy.sh) (__0.5.6__ is required)
- __Windows-only__: Run `npm install -g windows-build-tools` (this installs some build tools that aren't included by default on Windows)
- [Check and install any system packages for Revery](https://github.com/revery-ui/revery/wiki/Building-&-Installing)

## Build and Run

### Build the front-end

```sh
git clone https://github.com/onivim/oni2
cd oni2
esy install
esy bootstrap
esy build
```

> __NOTE:__ On Windows, you must __build from a shell running as administrator__. This is a requirement of esy because creating symlinks requires administrator permissions (more info here: https://github.com/esy/esy/issues/389).
### Build the textmate service

```sh
cd src/textmate_service
node install.js
npm run build
```

> __NOTE:__ The non-standard `node install.js` step instead of `npm install` is necessary because the script picks up our _vendored_ node binary - and the native dependencies for the textmate service rely on being built with the same version of node that it uses at runtime.

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
