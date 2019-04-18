[![Build Status](https://dev.azure.com/onivim/oni2/_apis/build/status/onivim.oni2?branchName=master)](https://dev.azure.com/onivim/oni2/_build/latest?definitionId=1?branchName=master)

# Oni 2

### Lightweight, Modal Code Editor

:warning: Pre-alpha - not yet usable! :warning: 

<p align="center">
  <img src="https://user-images.githubusercontent.com/13532591/53999860-e8e70780-40f9-11e9-8428-295adb18c4dd.gif" alt="Onivim 2" />
 </p>


## Introduction

Oni 2 is a reimagination of the [Oni](https://www.onivim.io) editor. Oni 2 aims to bring the _speed_ of Sublime, the _language integration_ of VSCode, and the _modal editing experience_ of (neo)vim together, in a single package.

Oni 2 is built in [`reason`](https://reasonml.github.io) using the [`revery`](https://github.com/revery-ui/revery) framework.

### Goals

- __Modern UX__ - an experience on par with modern code editors like VSCode and Atom
- __VSCode Plugin Support__ - use all of the features of VSCode plugins, including language servers and debuggers
- __Cross-Platform__ - works on Windows, OSX, and Linux
- __Batteries Included__ - works out of the box
- __Performance__ - no compromises: native performance, minimal input latency
- __Ease Learning Curve__ - Oni 2 should be comfortable for non-vimmers, too!

The goal of this project is to build an editor that doesn't exist today - the _speed_ of a native code editor like Sublime, the _power_ of modal editing, and the _rich tooling_ that comes with a lightweight editor like VSCode.

### Non-goals

- __VimL compatibility__ - we may not support all features of VimL plugins / configuration.

## Build

### Prerequisites

- Install [Git](https://git-scm.com/)
- Install [Esy](https://esy.sh) (__0.5.6__ is required)
- [Check and install any system packages for Revery](https://github.com/revery-ui/revery#building)

### Build

#### Build the front-end

- `git clone https://github.com/onivim/oni2`
- `cd oni2`
- `esy install`
- `esy bootstrap`
- `esy build`

#### Build the textmate service

- `cd src/textmate_service`
- `node install`
- `npm run build`

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

## Documentation

Coming soon

## Contributing

We'd :heart: help building Oni 2 - more info soon.

## License

Onivim 2 is licensed under the [Outrun Labs EULA 1.0](./Outrun-Labs-EULA-v1.0.md).

The TL;DR is:
- __Free__ for __non-commercial__ and __educational use__.
- __Commercial use__ requires the purchase of a license.
- You may not redistribute source code or binaries under a different license.

You can pre-order a commercial license here with a pay-what-you-want model: https://v2.onivim.io

As we get closer to shipping, we'll start bumping up the minimum required pre-order, until we settle on our full pricing model - $5/mo, $50/yr, or $99 lifetime.

__Anyone who has contributed financially to the project__ - via BountySource, Patreon, PayPal, or OpenCollective - __will automatically get a free lifetime license__ (we're still working out the logistics, but we got you!). Early adopters will never have to pay again, unless they wish to contribute more to the project.

Alternatively, you can contribute to the project through one of these avenues:
- [Patreon](https://www.patreon.com/onivim)
- [Open Collective](https://opencollective.com/oni)

Because of the support we've received from open source communities (both Neovim and Reason), we've decided also to __dual-license the code after 18 months__ - every commit, starting with [017c5131b4bba3006f726a3ef0f5a33028e059b5](https://github.com/onivim/oni2/commit/017c5131b4bba3006f726a3ef0f5a33028e059b5), will be dual-licensed via the __MIT License__ 18 months from that commit's date to `master`. For commit [017c5131b4bba3006f726a3ef0f5a33028e059b5](https://github.com/onivim/oni2/commit/017c5131b4bba3006f726a3ef0f5a33028e059b5), that means it would be dual-licensed with __MIT License__ on __10/18/2020__. We hope that this approach will bring us the best of worlds - the ability to have a commercially sustainable product, with high quality - as well as giving back to the open source communities.

If you wish to ship a product based on the Onivim 2 source code, and don't want to wait 18 months for a specific commit, we do offer an SDK license. Contact sales@outrunlabs.com for more info. However, it's possible you just want [Revery](https://github.com/revery-ui/revery), which is totally free and MIT licensed.

### Third-Party Code

Several dependencies have their own set of license terms here: [ThirdPartyLicenses.txt](ThirdPartyLicenses.txt)

Copyright 2019 Outrun Labs, LLC.
