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

#### Changelog generation

We use a script to generate the change log based on metadata picked up from the commit message, PR title and PR body. For the changelog to be generated successfully it's therefore important that these follow the exact format explained below.

##### PR title

PR titles should have the following format:

```
<type>(<scope>): <subject>
```

where `type` can be one of

* **feat:** A new feature
* **fix:** A bug fix
* **refactor:** A code change that neither fixes a bug nor adds a feature
* **perf:** A code change that improves performance
* **test:** Adding missing or correcting existing tests
* **docs:** Documentation only changes
* **chore:** Changes to the build process or auxiliary tools and libraries such as documentation generation
* **style:** Changes that do not affect the meaning of the code (white-space, formatting, missing semi-colons, etc)

`scope` is optional, but "encouraged". It is typically one of the "area" [issue labels](https://github.com/onivim/oni2/labels) or the name of a feature project.

`subject` should contain a succinct description of the change:

* use the imperative, present tense: "change" not "changed" nor "changes"
* don't capitalize first letter
* no dot (.) at the end

*Attribution: Adapted from [Angular's commit guidelines](https://github.com/angular/angular.js/blob/master/DEVELOPERS.md#commit-message-format)*

##### PR body

More details can be provided in a `changelog` code block in the body of the PR if a longer description is needed or if there are beaking changes:

    ```changelog
    <breaking>This change breaks my brain</breaking>
    
    Detailed description of the change
    ```

Text inside `<breaking>` tags will be listed in the changelog as separate items so that they can be emphasized. ALL breaking changes MUST be listed in this way. If in doubt, list it anyway. We'll remove it later in the process if it's considered insignificant.

The rest of the text will become the overall description of the change and can include anything considered text content in XML. Use [predefined entities](https://en.wikipedia.org/wiki/List_of_XML_and_HTML_character_entity_references#Predefined_entities_in_XML) in place of characters with special meaning in XML. And please do still try to be brief!

##### Commit message

*Note: this section is relevant only to maintainers who merge PRs. Contributors can safely ignore this.*

Commits messages pushed to `master` should have the following format:

```
<type>(<scope>): <subject> (#<PR number>)
```  

where `type` and `scope` is as described above. This is usually what Github generates automatically when squashing a PR, assuming the PR title was formatted correctly.

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

### Log levels

The following conventions are used to determine the priority level of log messages, based on [the conventions suggested by @dbuenzli/logs](https://erratique.ch/software/logs/doc/Logs/index.html#usage):

- __Error__ if it WILL cause unexpected behaviour
- __Warn__ if it MIGHT cause unexpected behaviour, but also might not, i.e. it's suspicious, but not a definite problem.
- __Info__ if it is both understandable AND actionable by the end-user
- __Debug__ otherwise
- __Trace__ if the output is overwhelming or just excessively detailed. Namespace filtering is expected at this level

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
