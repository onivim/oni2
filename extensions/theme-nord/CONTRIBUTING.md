# Contributing to Nord Visual Studio Code

Thanks for contributing to Nord Visual Studio Code!

This is a set of guidelines for contributing to Nord Visual Studio Code. Please take a moment to review this document in order to make the contribution process easy and effective for everyone involved.

Following these guidelines helps to communicate that you respect the time of the developers managing and developing this open source project. In return, they should reciprocate that respect in addressing your issue, assessing changes, and helping you finalize your pull requests.

As for everything else in the project, the contributions to Nord Visual Studio Code are governed by our [Code of Conduct][gh-coc]. By participating, you are expected to uphold this code. Please report unacceptable behavior at support@nordtheme.com or directly to one of the core team members via [email][gh-mailmap].

## Getting Started

Nord Visual Studio Code is an open source project and we love to receive contributions from the community! There are many ways to contribute, from [writing- and improving documentation and tutorials](#documentations), [reporting bugs](#bug-reports), [submitting enhancement suggestions](#enhancement-suggestions) which can be incorporated into this project and Nord Visual Studio Code itself by [submitting a pull request](#pull-requests).

The project development workflow and process uses [GitHub Issues][gh-issues]- and [Pull Requests][gh-pr] management to track issues and pull requests including multiple [issue templates][gh-issues-templates].

Before you continue with these contribution guidelines we highly recommend to read the awesome GitHub [Open Source Guides][os-guide] on how to [making open source contributions][os-guide-contrib].

### Repository Assignment

Nord is a large open source project which is [split up into multiple repositories][gh-profile-repo-search] with [support for many port projects][nt-ports] and is intentionally very modular. Each port project lives in its own repository.

**Please make sure to determine the correct repository before you continue!**
Contributions related to Nord belong to [Nord's repository][gh-nord] while contributions related to [this project][gh-nord-visual-studio-code] are welcome in the main repositories. This helps all core team members, committers and maintainer to process every contribution faster without organization overhead.

### Bug Reports

A bug is a _demonstrable problem_ that is caused by the code in the repository. This section guides you through submitting a bug report for Nord Visual Studio Code. Following these guidelines helps maintainers and the community understand your report, reproduce the behavior and find related reports.

**Do NOT report security vulnerabilities in public issues!** Please only contact one of the core team members or project owner in a responsible manner by [email][gh-mailmap] or via support@nordtheme.com. We will assess the issue as soon as possible on a best-effort basis and will give you an estimate for when we have a fix and release available for an eventual public disclosure.

- **Use the [GitHub Issue search][gh-issues]** — check if the issue has already been reported. If it has **and the issue is still open**, add a comment to the existing issue instead of opening a new one. If you find a closed issue that seems like it is the same thing that you are experiencing, open a new issue and include a link to the original issue in the body of your new one.
- **Determine [which repository the contribution belongs to](#repository-assignment).**
- **Check if the issue has been fixed** — try to reproduce it using the [latest version][gh-releases-latest] and [`develop`][gh-branch-develop] branch in the repository.
- **Isolate the problem** — ideally create a [MCVE](#mcve).

When you are creating a bug report, please provide as much detail and context as possible. Fill out on of [the required templates][gh-issues-template-bug], the information it asks helps maintainers to reproduce the problem and resolve issues faster.

- **Use a clear and descriptive title** for the issue to identify the problem.
- **Describe the exact steps which reproduce the problem** in as many details as possible.
- **Include screenshots and animated GIFs** if appropriate which show you following the described steps and clearly demonstrate the problem.
- **Provide specific examples to demonstrate the steps**. Include links to files or GitHub projects, or copy/pasteable snippets. If you are providing snippets in the issue, use [Markdown code blocks][ghh-markdown-code-blocks] or [attach files to the issue][ghh-attach-file].

If possible please provide more context by answering these questions:

- **Did the problem start happening recently** e.g. after updating to a new version of Nord Visual Studio Code or was this always a problem?
  - If the problem started happening recently, **can you reproduce the problem in an older version of Nord Visual Studio Code?**
  - What is the most recent version in which the problem does not happen?
- **Can you reliably reproduce the issue?** If not, please provide details about how often the problem happens and under which conditions it normally happens.

Please include details about your configuration and environment based on the [issue template][gh-issues-template-bug].

### Enhancement Suggestions

This section guides you through submitting an enhancement suggestion, including completely new features and minor improvements to existing functionality or any new [port project][nt-ports]. Following these guidelines helps maintainers and the community understand your suggestion and find related suggestions.

- **Use the [GitHub Issues search][gh-issues]** — check if this enhancement has already been suggested. If it has **and the issue is still open**, add your additions as comment to the existing issue instead of opening a new one.
- **Determine [which repository the contribution belongs to](#repository-assignment).**
- **Check if the enhancement has already been implemented** — use the [latest version][gh-releases-latest] and [`develop`][gh-branch-develop] branch to ensure that the feature or improvement has not already been added.
- **Provide a reduced show case** — ideally create a [MCVE](#mcve).

Before creating enhancement suggestions, please check if your idea fits with the scope and provide as much detail and context as possible using a structured layout like the [the issue template][gh-issues-template-enhancement].

- **Use a clear and descriptive title** for the issue to identify the suggestion.
- **Provide a step-by-step description of the suggested enhancement** in as many details as possible and provide use-cases.
- **Provide examples to demonstrate the need of an enhancement**. Include copy/pasteable snippets which you use in those examples, use [Markdown code blocks][ghh-markdown-code-blocks] or [attach files to the issue][ghh-attach-file].
- **Describe the current behavior** and **explain which behavior you expected to see instead** and why.
- **Explain why this enhancement would be useful** to most Nord Visual Studio Code users.
- **Maybe list some other projects where this enhancement exists.**

### Pull Requests

This section guides you through submitting an pull request. Following these guidelines helps maintainers and the community to better understand your code.

**Please [suggest an enhancement](#enhancement-suggestions) or [report a bug](#bug-reports) first before embarking on any significant pull request** (e.g. implementing features, refactoring code, fixing a bug), otherwise you risk spending a lot of time working on something that the core team members and project owner might not want to merge into the project.

When you are submitting an pull request, please provide as much detail and context as possible. Fill out [the required template][gh-issues-template-pr] to help maintainers to understand your submitted code.

- **Use a clear and descriptive title for the pull request**
- **Do not include issue numbers in the pull request title** but fill in the metadata section at the top of the [required pull request template][gh-issues-template-pr] making use of the [GitHub issue keywords][ghh-issue-keywords] to link to specific [enhancement suggestions](#enhancement-suggestions) or [bug reports](#bug-reports).
- **Include screenshots and animated GIFs** if appropriate which show you following the described steps and clearly demonstrate the change.
- **Make sure to follow the [JavaScript](#javascript-style-guide), [Markdown](#markdown-style-guide) and [Git](#git-style-guide) style guides**.
- **Remain focused in scope and avoid to include unrelated commits**.
- **Features and improvements should always be accompanied with tests and documentation**. If the pull request improves the performance consider to include a benchmark test, optimally including a chart.
- **Lint and test before submitting the pull request**.
- **Make sure to create the pull request from a [topic branch][git-docs-branching-workflows]**.

**All pull requests must be send against the [`develop`][gh-branch-develop] branch** - Please read the [branch organization](#branch-organization) section below for details about the branching model.

### Documentations

Nord Visual Studio Code's documentation consists of guides, which helps users to learn about the project, and the project docs, which serves as a reference.

You can help improve the docs and guides by making them more coherent, consistent or readable, adding missing information, correcting factual errors, fixing typos, bringing them up to date when there are differences to the latest version. This can be done by submitting a [enhancement suggestion](#enhancement-suggestions) and then opening a [pull request](#pull-requests) for it.

## Branch Organization

Nord Visual Studio Code uses the [gitflow][gitflow] branching model. The repository consists of two core branches with an infinite development lifecycle:

- `master` - The source code of `HEAD` always reflects a tagged release version.
- `develop` - The default branch where the source code of `HEAD` always reflects a state with the latest development state.

**All [pull requests](#pull-requests) for the limited development lifecycle _story_/_topic_ branches must be send against the `develop` branch**.

<!-- lint disable no-heading-punctuation -->

## How else can I help?

<!-- lint enable no-heading-punctuation -->

### Improve Issues

Some issues are created with missing information, not reproducible, or plain invalid. You can help to make it easier for maintainer to understand and resolve them faster. since handling issues takes a lot of time that could rather spend on writing code.

### Give Feedback On Issues and Pull Requests

We're always looking for more opinions on discussions in issues and pull request reviews which is a good opportunity to influence the future direction of Nord Visual Studio Code.

The [question][gh-issues-label-question] issue label is a good place to find ongoing discussions and questions.

## Style Guides

Every major open source project has its own style guide, a set of standards and conventions for the writing and design of code, documentations and Git commit messages. It is much easier to understand a large code base when all the code in it is in a consistent style.

A style guide establishes and enforces style to improve the intelligibility and communication within the project community. It ensures consistency and enforces best practice in usage and language composition.

### JavaScript Style Guide

Nord Visual Studio Code adheres to the [Arctic Ice Studio JavaScript Style Guide][gh-styleguide-javascript].

[![][gh-styleguide-javascript-badge]][gh-styleguide-javascript]

### Markdown Style Guide

Nord Visual Studio Code adheres to the [Arctic Ice Studio Markdown Style Guide][gh-styleguide-markdown].

[![][gh-styleguide-markdown-badge]][gh-styleguide-markdown]

### Git Commit Messages

A well-crafted Git commit message is the best way to communicate _context_ about a change to the maintainers. The code will tell what changed, but only the commit message can properly tell why. Re-establishing the context of a piece of code is wasteful. We can't avoid it completely, so our efforts should go to reducing it as much as possible.

Nord Visual Studio Code adheres to the [Arctic Ice Studio Git Style Guide][gh-styleguide-git].

[![][gh-styleguide-git-badge]][gh-styleguide-git]

The style guide assumes that you are familiar with the [gitflow][gitflow] branching model.

## MCVE

A Minimal, Complete, and Verifiable Example.

When [reporting a bug](#bug-reports), sometimes even when [suggesting enhancements](#enhancement-suggestions), the issue can be processed faster if you provide code for reproduction. That code should be…

- …Minimal – Use as little code as possible that still produces the same behavior
- …Complete – Provide all parts needed to reproduce the behavior
- …Verifiable – Test the code you're about to provide to make sure it reproduces the behavior

A MCVE is a common practice like on [Stack Overflow][stackoverflow-mcve] and sometimes it is also called [SSCCE][sscce], a _Short, Self Contained, Correct (Compilable), Example_.

The recommended way for GitHub based projects is to create it as [Gist][gh-gist] or new repository, but of course you can [attach it to issues and pull requests as files][ghh-attach-file], use any free code paste- or file hosting service or paste the code in [Markdown code blocks][ghh-markdown-code-blocks] into the issue.

### Minimal

The more code there is to go through, the less likely developers can understand your enhancement or find the bug. Streamline your example in one of two ways:

- **Restart from scratch**. Create new code, adding in only what is needed to demonstrate the behavior and is also useful if you can't post the original code publicly for legal or ethical reasons.
- **Divide and conquer**. When you have a small amount of code, but the source of the bug is entirely unclear, start removing code a bit at a time until the problem disappears – then add the last part back and document this behavior to help developers to trace- and debug faster.

#### Minimal and readable

Minimal does not mean terse – don't sacrifice communication to brevity. Use consistent naming and indentation following the [style guides](#style-guides), and include comments if needed to explain portions of the code.

### Complete

Make sure all resources and code necessary to reproduce the behavior is included. The problem might not be in the part you suspect it is, but another part entirely.

### Verifiable

To entirely understand your enhancement or bug report, developers will need to verify that it _exists_:

- **Follow the contribution guidelines regarding the description and details**. Without information developers won't be able to understand and reproduce the behavior.
- **Eliminate any issues that aren't relevant**. Ensure that there are no compile-time errors.
- **Make sure that the example actually reproduces the problem**. Sometimes the bug gets fixed inadvertently or unconsciously while composing the example or does not occur when running on fresh machine environment.

## Versioning

Nord Visual Studio Code follows the [Semantic Versioning Specification][semver] (SemVer). We release patch versions for bug fixes, minor versions for enhancements like new features and improvements, and major versions for any backwards incompatible changes. Deprecation warnings are introduced for breaking changes in a minor version so that users learn about the upcoming changes and migrate their code in advance.

Every significant change is documented in the [changelog][gh-changelog].

## Credits

Thanks for the inspirations and attributions to GitHub's [Open Source Guides][os-guide] and various contribution guides of large open source projects like [Atom][gh-atom-contrib], [React][react-contrib] and [Ruby on Rails][ruby-on-rails-contrib].

[gh-atom-contrib]: https://github.com/atom/atom/blob/master/CONTRIBUTING.md
[gh-branch-develop]: https://github.com/arcticicestudio/nord-visual-studio-code/tree/develop
[gh-changelog]: https://github.com/arcticicestudio/nord-visual-studio-code/blob/develop/CHANGELOG.md
[gh-coc]: https://github.com/arcticicestudio/nord-visual-studio-code/blob/develop/CODE_OF_CONDUCT.md
[gh-gist]: https://gist.github.com
[gh-issues]: https://github.com/arcticicestudio/nord-visual-studio-code/issues
[gh-issues-label-question]: https://github.com/arcticicestudio/nord-visual-studio-code/labels/type-question
[gh-issues-template-bug]: https://github.com/arcticicestudio/nord-visual-studio-code/blob/develop/.github/ISSUE_TEMPLATE/bugs.md
[gh-issues-template-enhancement]: https://github.com/arcticicestudio/nord-visual-studio-code/blob/develop/.github/ISSUE_TEMPLATE/enhancement.md
[gh-issues-template-pr]: https://github.com/arcticicestudio/nord-visual-studio-code/blob/develop/.github/PULL_REQUEST_TEMPLATE.md
[gh-issues-templates]: https://github.com/arcticicestudio/nord-visual-studio-code/issues/new/choose
[gh-mailmap]: https://github.com/arcticicestudio/nord-visual-studio-code/blob/develop/.mailmap
[gh-nord]: https://github.com/arcticicestudio/nord
[gh-nord-visual-studio-code]: https://github.com/arcticicestudio/nord-visual-studio-code
[gh-pr]: https://github.com/arcticicestudio/nord-visual-studio-code/pulls
[gh-profile-repo-search]: https://github.com/arcticicestudio?&tab=repositories&q=nord
[gh-releases-latest]: https://github.com/arcticicestudio/nord-visual-studio-code/releases/latest
[gh-styleguide-git]: https://github.com/arcticicestudio/styleguide-git
[gh-styleguide-git-badge]: https://raw.githubusercontent.com/arcticicestudio/styleguide-git/develop/assets/styleguide-git-banner-typography-badge.svg?sanitize=true
[gh-styleguide-javascript]: https://github.com/arcticicestudio/styleguide-javascript
[gh-styleguide-javascript-badge]: https://raw.githubusercontent.com/arcticicestudio/styleguide-javascript/develop/assets/styleguide-javascript-banner-typography-badge.svg?sanitize=true
[gh-styleguide-markdown]: https://github.com/arcticicestudio/styleguide-markdown
[gh-styleguide-markdown-badge]: https://raw.githubusercontent.com/arcticicestudio/styleguide-markdown/develop/assets/styleguide-markdown-banner-typography-badge.svg?sanitize=true
[ghh-attach-file]: https://help.github.com/articles/file-attachments-on-issues-and-pull-requests
[ghh-issue-keywords]: https://help.github.com/articles/closing-issues-using-keywords
[ghh-markdown-code-blocks]: https://help.github.com/articles/basic-writing-and-formatting-syntax
[git-docs-branching-workflows]: https://git-scm.com/book/en/v2/Git-Branching-Branching-Workflows
[gitflow]: http://nvie.com/posts/a-successful-git-branching-model
[nt-ports]: https://nordtheme.com/ports
[os-guide]: https://opensource.guide
[os-guide-contrib]: https://opensource.guide/how-to-contribute
[react-contrib]: https://facebook.github.io/react/contributing/how-to-contribute.html
[ruby-on-rails-contrib]: http://guides.rubyonrails.org/contributing_to_ruby_on_rails.html
[semver]: https://semver.org
[sscce]: http://sscce.org
[stackoverflow-mcve]: https://stackoverflow.com/help/mcve
