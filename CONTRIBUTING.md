# How to Contribute

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

## Pull Requests

Before we can accept a pull request from you, you'll need to sign a a [Contributor License Agreement (CLA)](https://gist.github.com/bf98297731dd69b9b580ca1d7fd2b90e). It is an automated process and you'll be guided
through it the first time you open a PR.

To enable us to quickly review and accept your pull requests, follow these guidelines:
- Always create __one pull request per issue__ and __link the issue in the pull request__. Never merge multiple requests into one.
- Keep code changes __as small as possible__. Break large PRs or features into smaller, incremental PRs were possible.
- Make our maintainer's life easy and keep changes __as simple as possible.__
- Avoid pure formatting changes for code that has not been otherwise modified.
- Include tests whenever possible.
- Include benchmarks whenever possible.

To avoid duplicate work, if you decide to start working on an issue, please leave a comment on the issue.

### Where to Contribute

Check out the [full issues list](https://github.com/onivim/oni2/issues) for ideas of where to start. Note that just because an issue exists does not mean we will accept a PR for it.

There are several reason we may not accept a pull request, like:
- __Performance__ - Onivim 2 is lightweight and fast. Changes should not introduce performance regressions.
- __User Experience__ - The UX should be smooth, polished, consistent, and not cluttered.
- __Architectural__ - Maintainers must approve any architectural impact or change.
- __Maintenance Burden__ - If a PR would incur a maintenance burden on the maintainers, it will be rejected.

To improve the chances to get a pull request merged, you should select an issue that is labeled with [bug](https://github.com/onivim/oni2/issues?q=is%3Aissue+is%3Aopen+label%3Abug) or [help wanted](https://github.com/onivim/oni2/issues?q=is%3Aissue+is%3Aopen+label%3A%22help+wanted%22).

In addition, Onivim 2 is built on [Revery](https://github.com/revery-ui/revery) - any work or improvements there will directly improve Onivim 2, as well!

## Discussion Etiquette

We strictly enforce a [Code of Conduct](./CODE_OF_CONDUCT.md) and have a zero-tolerance policy towards infractions. Be considerate to others, and try to be courteous and professional at all times.
