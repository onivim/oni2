---
id: rescript
title: ReScript
sidebar_label: ReScript
---

Onivim does not come with out-of-the-box support for [ReScript](https://rescript-lang.org), but the [`rescript-vscode`](https://open-vsx.org/extension/chenglou92/rescript-vscode) extension can be used with Onivim.

![rescript-completion](https://user-images.githubusercontent.com/13532591/109860702-5065be00-7c13-11eb-8ebe-6fcbe8d8bd91.png)

## Setup

1) Follow the [installation](https://rescript-lang.org/docs/manual/latest/installation) steps for ReScript, and make sure that you can build and run the project.

2) Install the `rescript-vscode` extension:

![rescript-install](https://user-images.githubusercontent.com/13532591/109860695-4f349100-7c13-11eb-9ef4-aa2078a384bc.png)

> Alternatively, the extension can be installed via the CLI: `oni2 --install-extension chenglou92.rescript-vscode`

3) Open Onivim in the root folder of your project.

If everything goes well, the extension will prompt you to build your project:

![rescript-build-prompt](https://user-images.githubusercontent.com/13532591/109860689-4d6acd80-7c13-11eb-9aac-41d87eb4c1ad.png)

## Usage

As of writing, the `rescript-vscode` extension supports:

- Syntax highlighting
- Hover
- Completion

## More Info

- [ReScript Installation](https://rescript-lang.org/docs/manual/latest/installation)
- [rescript-vscode](https://github.com/rescript-lang/rescript-vscode)

