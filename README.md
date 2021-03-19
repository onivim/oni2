[![Build Status](https://dev.azure.com/onivim/oni2/_apis/build/status/onivim.oni2?branchName=master)](https://dev.azure.com/onivim/oni2/_build/latest?definitionId=1?branchName=master)
[![Discord](https://img.shields.io/discord/417774914645262338.svg)](https://discord.gg/7maEAxV)

# Onivim 2

### Lightweight, Modal Code Editor

:warning: Currently in alpha - may have bugs or instability - use with caution :warning: 

<p align="center">
  <img width="800" alt="Onivim 2" src="https://user-images.githubusercontent.com/13532591/109881233-c88cad80-7c2c-11eb-83a1-67a7adcbbd7e.png">
 </p>


## Introduction

Onivim 2 is a reimagination of the [Oni](https://www.onivim.io) editor. Onivim 2 aims to bring the _speed_ of Sublime, the _language integration_ of VSCode, and the _modal editing experience_ of Vim together, in a single package.

Onivim 2 is built in [`reason`](https://reasonml.github.io) using the [`revery`](https://github.com/revery-ui/revery) framework.

Onivim 2 uses [`libvim`](https://github.com/onivim/libvim) to manage buffers and provide authentic modal editing, and features a fast, native front-end. In addition, Onivim 2 leverages the VSCode Extension Host process in its entirety - meaning, eventually, complete support for VSCode extensions and configuration.

### Goals

- __Modern UX__ - an experience on par with modern code editors like VSCode and Atom
- __VSCode Plugin Support__ - use all of the features of VSCode plugins, including language servers and debuggers
- __Cross-Platform__ - works on Windows, OSX, and Linux
- __Batteries Included__ - works out of the box
- __Performance__ - no compromises: native performance, minimal input latency
- __Easy to Learn__ - Onivim 2 should be comfortable for non-vimmers, too!

The goal of this project is to build an editor that doesn't exist today - the _speed_ of a native code editor like Sublime, the _power_ of modal editing, and the _rich tooling_ that comes with a lightweight editor like VSCode.

### Non-goals

- __100% VimL compatibility__ - we may not support all features of VimL plugins / configuration.

## Documentation

- Onivim User Manual
  - [Getting Started](https://onivim.github.io/docs/getting-started/why-onivim)
  - [Basic Usage](https://onivim.github.io/docs/using-onivim/moving-in-onivim)
  - [FAQ](https://onivim.github.io/docs/other/faq)

## Contributing

We'd :heart: help building Onivim 2!

If you are interested in fixing issues and contributing directly to the code, please see the [How to Contribute](https://onivim.github.io/docs/for-developers/contributing) document, which includes the following:

- [How to build and run from source](https://onivim.github.io/docs/for-developers/building)
- [Pull Request Guidelines](https://onivim.github.io/docs/for-developers/contributing)

Please also see our [Code of Conduct](./CODE_OF_CONDUCT.md).

## Feedback

- [Tweet us](https://twitter.com/oni_vim) with feedback
- Come visit us on [Discord](https://discord.gg/7maEAxV)
- Join the discussion on [Reddit](https://reddit.com/r/onivim)
- [File an issue](https://github.com/onivim/oni2/issues) on GitHub

## License

Onivim 2 is licensed under the [Outrun Labs EULA 1.1](./Outrun-Labs-EULA-v1.1.md).

The TL;DR is:
- __Free__ for __non-commercial__ and __educational use__.
- __Commercial use__ requires the purchase of a license.
- You may not redistribute source code or binaries under a different license.

You can pre-order a commercial license here: https://v2.onivim.io

As we get closer to shipping our MVP, we'll increase the pre-order price, until we settle on our full pricing model.

> __NOTE:__ Anyone who contributed financially to Onivim v1 via our past funding sources (BountySource, Patreon, PayPal, OpenCollective) - gets a lifetime license. If you haven't received your license key, and you contributed previously, please contact me at bryphe@outrunlabs.com (or feel free to reach out at [Twitter](https://twitter.com/oni_vim) or our [Discord](https://discord.gg/7maEAxV)). __Thank you for supporting the project!__

Alternatively, you can contribute to the project through [Patreon](https://www.patreon.com/onivim), which helps us with ongoing costs.

#### 'Time-Delay' Dual License

Because of the support we've received from open source communities, we've decided to __dual-license the code after 18 months__ - every commit, starting with [017c513](https://github.com/onivim/oni2/commit/017c5131b4bba3006f726a3ef0f5a33028e059b5), will be dual-licensed via the __MIT License__ 18 months from that commit's date to `master`. For commit [017c513](https://github.com/onivim/oni2/commit/017c5131b4bba3006f726a3ef0f5a33028e059b5), as it was committed to `master` on __4/18/2019__ that means it would be dual-licensed with __MIT License__ on __10/18/2020__. 

For convenience, we will maintain an [oni2-mit](https://github.com/onivim/oni2-mit) repo containing the MIT-licensed code. The first commit to that repo will be on __July 2, 2020__.

Any external contributions to the project from outside Outrun Labs, LLC will not be subject to this 'time-delay' - they'll be dual-licensed immediately under the MIT License.

We hope that this approach will bring us the best of worlds - the ability to have a commercially sustainable product, with high quality - as well as giving back to the open source communities by having our work eventually end up in the open, and ensuring that external contributions are always open source.

#### Third-Party Code

Several dependencies have their own set of license terms here: [ThirdPartyLicenses.txt](ThirdPartyLicenses.txt)

Copyright (c) 2019-2020 Outrun Labs, LLC.
