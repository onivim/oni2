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

The goal of this project is to build an editor that doesn't exist today - the _speed_ of a native code editor like Sublime, the _power_ of modal editing, and the _rich tooling_ that comes with a lighweight editor like VSCode.

### Non-goals

- __VimL compatibility__ - we may not support all features of VimL plugins / configuration.

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
- `npm run build`

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

Onivim 2 is currently licensed under the [CC-BY-NC-4.0](https://creativecommons.org/licenses/by-nc/4.0/legalcode) license.

This means that Onivim 2 is __free to use__ for __non-commercial__ or __educational__ uses. 

> __NOTE:__ We're reviewing our license terms with a lawyer, so they may change slightly (it turns out the CC-BY-NC-4.0 isn't the perfect fit for software - we might need a more official EULA).

For __commercial use licenses__, we're still working through the details - but once Onivim 2 has reached "MVP" (target - end of May), we'll transition from crowdfunding to selling commercial licenses. Our current planned price point is $10/month.

However, until that time, __we're offering anyone who donates to the project - any dollar amount__ - an individual, perpetual-use commercial license. We don't want early adopters to have to pay again, ever, as we launch Onivim 2 - we truly appreciate the support as we transition to a commercial offering, and we know it's a leap of faith to back an early-stage project!

More information about this decision in this [Reddit Thread: Question About Oni 2 License](https://www.reddit.com/r/neovim/comments/ae7ef6/question_about_oni_2_license/).

You can donate to the project through one of these avenues:
- [Patreon](https://www.patreon.com/onivim)
- [Open Collective](https://opencollective.com/oni)
- Or, if you don't want to get stuck with a recurring payment, [PayPal](https://www.paypal.me/bryphe/10). 

Several dependencies have their own set of license terms here: [ThirdPartyLicenses.txt](ThirdPartyLicenses.txt)

Copyright 2019 Outrun Labs, LLC.
