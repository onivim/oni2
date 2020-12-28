---
name: Bug Report
about: Report a bug that is caused by the code in this repository
---

<!-- Click on the "Preview" tab to render the instructions in a more readable format -->

> **Please read the [contribution guidelines](https://github.com/arcticicestudio/nord-visual-studio-code/blob/develop/CONTRIBUTING.md) before filling out this issue template**.

## Prerequisites

This section and the instructions in the sections below are only part of this issue template. Please ensure to **delete this whole section, all pre-filled instructions of the sections below and sections you have not filled out before submitting** to ensure a clear structure and overview.

Please do your best to provide as much information as possible and use a clear and descriptive title for your bug report to help maintainers and the community understand and reproduce the behavior, find related reports and to resolve the ticket faster.

- **Ensure the bug has not already been reported by using the [GitHub Issues search](https://github.com/arcticicestudio/nord-visual-studio-code/issues)** — check if this enhancement has already been suggested. If it has **and the issue is still open**, add your additions as comment to the existing issue instead of opening a new one. If you find a closed issue that seems to be similar to this one, include a link to the original issue in the [metadata head](#metadata-head) section of this issue.
- **Ensure your contribution belongs to the correct [main or port project repository](https://github.com/arcticicestudio?&tab=repositories&q=nord).**
- **Ensure the bug is reproducible and has not already been fixed** — use the [latest version](https://github.com/arcticicestudio/nord-visual-studio-code/releases/latest) and [`develop`](https://github.com/arcticicestudio/nord-visual-studio-code/tree/develop) branch.

## Metadata Head

The metadata head can be added to the top of the issue as [Markdown text quote](https://help.github.com/articles/basic-writing-and-formatting-syntax) containing the the ID of other related issues.

> Related issues:

## Description

Describe the bug as in many relevant details as possible with a clear and concise description. Ensure to fill in the [steps to reproduce](#steps-to-reproduce) it.

### Steps to Reproduce

1. Step One
2. Step Two
3. ...

### Expected Behavior

What you expect to happen?

### Actual Behavior

What actually happens?

## Example

Provide a [MCVE - The Minimal, Complete, and Verifiable Example](https://github.com/arcticicestudio/nord-visual-studio-code/blob/develop/CONTRIBUTING.md#mcve)

**This is a optional section, but it can drastically increase the speed at which this issue can be processed since it takes away the time-consuming reconstruction to reproduce the bug.**
The recommended way is to upload it as [Gist](https://gist.github.com) or new repository to GitHub, but of course you can [attach it to this issue](https://help.github.com/articles/file-attachments-on-issues-and-pull-requests), use any free file hosting service or paste the code in [Markdown code blocks](https://help.github.com/articles/basic-writing-and-formatting-syntax) into this issue.

## Environment and Versions

- What is the version of _Nord Visual Studio Code_ you are running?
- What is the name and the version of your OS?
  - Have you tried to reproduce it on different OS environments and if yes is the behavior the same for all?
- If the problem is related to the runtime of the project (e.g. [Node.js](https://nodejs.org), [Go](https://golang.org) or [Java](https://java.com)) please provide the version you're running.
  - Are you using any additional CLI arguments to run the project?
- What is the version of the build tool (e.g. [npm](https://www.npmjs.com), [dep](https://golang.github.io/dep) or [Gradle](https://gradle.org)) you are running?
  - Are you using any additional CLI arguments to start the build tool task/script other than defined by the project?

If you've installed [Node.js](https://nodejs.org) on your system you can run [envinfo](https://www.npmjs.com/package/envinfo) via [npx](https://blog.npmjs.org/post/162869356040/introducing-npx-an-npm-package-runner) which will print environment information that help the the community to better reproduce the bug.

Run the following command **from within the project root** and paste the output in the code block below: `npx envinfo --system --system --IDEs --languages --binaries --markdown --clipboard`

```md
Paste output of the command here.
```

## Stack Trace and Error Messages

```raw
Paste the full stack trace, error messages or the logfile here.
```

... or [attach them as files](https://help.github.com/articles/file-attachments-on-issues-and-pull-requests) to this issue.

## Additional Context

Add any other context, screenshots or screencasts which are relevant for this issue.

## References

Add any other references and links which are relevant for this issue.

## Potential Solution

Maybe include the lines of code that you have identified as causing the bug or references to other projects where this bug has already been reported.
