## How to contribute to Onivim

### Changelog Generation

We use a script to generate the change log based on metadata picked up from the commit message, and pull request title and body. For the changelog to be generated successfully it's therefore important that these follow the exact format explained below.

## Pull Requests

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