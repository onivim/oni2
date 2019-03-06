[![Build Status](https://dev.azure.com/onivim/oni2/_apis/build/status/onivim.oni2?branchName=master)](https://dev.azure.com/onivim/oni2/_build/latest?definitionId=1?branchName=master)

# Oni 2

### Lightweight, Modal Code Editor

:warning: Pre-alpha - not yet usable! :warning: 

## Introduction

Oni 2 is a reimagination of the [Oni](https://onivim.io) editor. Oni 2 aims to bring the _speed_ of Sublime, the _language integration_ of VSCode, and the _modal editing experience_ of (neo)vim together, in a single package.

Oni 2 is built in [`reason`](https://reasonml.github.io) using the [`revery`](https://github.com/revery-ui/revery) framework.

## Goals

- __Modern UX__ - an experience on par with modern code editors like VSCode and Atom
- __VSCode Plugin Support__ - use all of the features of VSCode plugins, including language servers and debuggers
- __Cross-Platform__ - works on Windows, OSX, and Linux
- __Batteries Included__ - works out of the box
- __Performance__ - no compromises: native performance, minimal input latency
- __Ease Learning Curve__ - Oni 2 should be comfortable for non-vimmers, too!

The goal of this project is to build an editor that doesn't exist today - the _speed_ of a native code editor like Sublime, the _power_ of modal editing, and the _rich tooling_ that comes with a lighweight editor like VSCode.

## Build

### Prerequisites

- Install [Git](https://git-scm.com/)
- Install [Esy](https://esy.sh) (__0.5.6__ is required)

##### `macOS`

- `brew install cmake`
- `brew install libpng ragel`

##### `Linux`

Install the following packages with your package manager of choice:
- `cmake`
- `ragel`

### Build

#### Build the front-end

- `git clone https://github.com/onivim/oni2`
- `esy install`
- `esy bootstrap`
- `esy build`

#### Build the textmate service

- `cd src/textmate_service`
- `node install.js`
- `npm run build

### Run

- `esy run`

### Tests

- `esy test`

### Benchmarks

- `esy '@bench' install`
- `esy '@bench' build`
- `esy '@bench' x oni-bench`

## Documentation

Coming soon

## Contributing

We'd :heart: help building Oni 2 - more info soon.

## License

Oni 2 is licensed under the [CC-BY-NC-4.0](https://creativecommons.org/licenses/by-nc/4.0/legalcode) license.

This means that Oni 2 is __free to use__ for __non-commercial__ or __educational__ uses. You are free to distribute and modify Oni 2 for personal use, as long as this README and License are included unmodified.

Several dependencies have their own set of license terms here: [ThirdPartyLicenses.txt](ThirdPartyLicenses.txt)

Copyright 2019 Outrun Labs, LLC.
