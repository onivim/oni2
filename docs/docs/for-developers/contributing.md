---
id: contributing
title: How to Contribute
sidebar_label: How to Contribute
---

## Where to Contribute

### Logging Issues

You can help us by logging any issues you find, as well as 'thumbs-upping' any issues relevant to you. As a small team, prioritization is critical, so knowing which issues are impactful for many users can help us with that prioritization.

### Pull Requests

Check out the [full issues list](https://github.com/onivim/oni2/issues) for ideas of where to start. Note that just because an issue exists does not mean we will accept a PR for it.

There are several reason we may not accept a pull request, like:
- __Performance__ - Onivim 2 is lightweight and fast. Changes should not introduce performance regressions.
- __User Experience__ - The UX should be smooth, polished, consistent, and not cluttered.
- __Architectural__ - Maintainers must approve any architectural impact or change.
- __Maintenance Burden__ - If a PR would incur a maintenance burden on the maintainers, it will be rejected.

To improve the chances to get a pull request merged, you should select an issue that is labeled with [bug](https://github.com/onivim/oni2/issues?q=is%3Aissue+is%3Aopen+label%3Abug) or [help wanted](https://github.com/onivim/oni2/issues?q=is%3Aissue+is%3Aopen+label%3A%22help+wanted%22).

In addition, Onivim 2 is built on [Revery](https://github.com/revery-ui/revery) - any work or improvements there will directly improve Onivim 2, as well!

### Submitting a Pull Request

Before we can accept a pull request from you, you'll need to sign a a [Contributor License Agreement (CLA)](https://gist.github.com/bf98297731dd69b9b580ca1d7fd2b90e). It is an automated process and you'll be guided
through it the first time you open a PR.

To enable us to quickly review and accept your pull requests, follow these guidelines:
- Always create __one pull request per issue__ and __link the issue in the pull request__. Never merge multiple requests into one.
- Keep code changes __as small as possible__. Break large PRs or features into smaller, incremental PRs where possible.
- Make our maintainer's life easy and keep changes __as simple as possible.__
- Avoid pure formatting changes for code that has not been otherwise modified.
- Include tests whenever possible.
- Include benchmarks whenever possible.

To avoid duplicate work, if you decide to start working on an issue, please leave a comment on the issue.

### Branch Naming

We recommend this scheme for naming branches: `<type>/<area>/<description>`

`type` is one of:
- `bugfix` - a change that fixes a bug
- `feature` - a change that adds new functionality
- `doc` - a change that modifies the documentation
- `refactoring` - a code change that does not fix a bug or change a feature
- `dependency` - a change to bring in a new dependency

`area` corresponds to our [Area Labels](https://github.com/onivim/oni2/labels?utf8=%E2%9C%93&q=A+-) 

`description` is just a short, hyphen-delimited blurb to very briefly describe the change.

Some examples:
- `bugfix/vim/fix-gd-crash`
- `feature/exthost/go-to-definition`
- `refactoring/editor-component/remove-duplication`

## Discussion Etiquette

We strictly enforce a [Code of Conduct](https://github.com/onivim/oni2/blob/master/CODE_OF_CONDUCT.md) and have a zero-tolerance policy towards infractions. Be considerate to others, and try to be courteous and professional at all times.
