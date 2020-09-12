## Submitting a Pull Request

Before we can accept a pull request from you, you'll need to sign a [Contributor License Agreement (CLA)](https://gist.github.com/bryphe/bf98297731dd69b9b580ca1d7fd2b90e). It is an automated process and you'll be guided through it the first time you open a PR.

To enable us to quickly review and accept your pull requests, follow these guidelines:

* Always create **one pull request per issue** and **link the issue in the pull request**. Never merge multiple requests into one.
* Keep code changes **as small as possible**. Break large PRs or features into smaller, incremental PRs where possible.
* Make our maintainer's life easy and keep changes **as simple as possible**.
* Avoid pure formatting changes for code that has not been otherwise modified.
* Include tests whenever possible
* Include benchmarks whenever possible.

To avoid duplicate work, if you decide to start working on an issue, please leave a comment on the issue.

### Changelog Generation

We use a script to generate the change log based on metadata picked up from the commit message, and pull request title and body. For the changelog to be generated successfully it's therefore important that these follow the exact format explained below.

### Pull Request Title

Pull request titles should have the following format:
    **<type>(<scope>): <subject>**

where _type_ can be one of:

* **feat:** A new feature
* **fix:** A bug fix
* **refactor:** A code change that neither fixes a bug nor adds a feature
* **perf:** A code change that improves performance
* **test:** Correcting existing tests or adding new ones
* **docs:** Documentation only changes
* **chore:** Changes to the build process or auxiliary tools and libraries such as documentation generation

_scope_ is optional, but "encouraged". It is typically one of the ["area" issue labels](https://github.com/onivim/oni2/labels?utf8=%E2%9C%93&q=A+-) or the name of a feature project. Supplementary information, like associated issues, can be added to the scope following a `/`. For example, a PR that fixes issue `#123` regarding vim could have the type and scope `fix(vim/#123)`.

_subject_ should contain a succinct description of the change:

* use the imperative, present tense: "change" not "changed" nor "changes"
* don't capitalize the first letter
* no dot (.) at the end

Do NOT add temporary tags like `[WIP]` to the title. Use GitHub's draft function and/or labels.

Some examples:

* feat(scm): add support for multiple providers
* fix(vim/#123): `get_op_type` error when using `gcc`
* refactor(editor): remove duplication
* chore(ci): fix build failure on CentOS VM

_Attribution: Adapted from [Angular's commit guidelines](https://github.com/angular/angular.js/blob/master/DEVELOPERS.md#commit-message-format)_

### Pull Request Body

More details can be provided in a `changelog` code block in the body of the pull request if a longer description is needed or if there are beaking changes:

    ```changelog
    Detailed description of the change
    Can span multiple lines

    <breaking>This change breaks my brain</breaking>
    ```

Text inside `<breaking>` tags will be listed in the changelog as seperate items so that they can be emphasized. ALL breaking changes MUST be listed in this way. If in doubt, list it anyway. We'll remove it later in the process if it's considered insignificant.

The rest of the text will become the overall description of the change and can include anything considered text content in XML. Use [predefined entities](https://en.wikipedia.org/wiki/List_of_XML_and_HTML_character_entity_references#Predefined_entities_in_XML) in place of characters with special meaning in XML. Please try to be brief!