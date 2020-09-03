---
id: extensions
title: Extensions
sidebar_label: Extensions
---

## VSCode Extensions

Onivim is capable of loading VSCode extensions, and supports functionality such as:

- Code completion
- Diagnostics
- Go-to definition
- Hover
- Signature Help
- Status bar items
- Source control
- Themes
- Syntax Highlights

The VSCode marketplace is proprietary - so Onivim uses the [Open VSX](https://open-vsx.org) marketplace: an open, vendor-neutral repository of VSCode extensions.

There are more details about the Open VSX project in the [Eclipse Open VSX article](https://www.eclipse.org/community/eclipse_newsletter/2020/march/1.php)

### Install via the UI

![image](https://user-images.githubusercontent.com/13532591/88987848-18b52300-d28c-11ea-86fa-e20ed1558fb5.png)

> __HINT:__ Use [sneak mode](./../using-onivim/moving-in-onivim#sneak-mode) to avoid the mouse.

### Install via the CLI

Extensions can be installed via the CLI, using the `--install-extension` argument:

```sh
oni2 --install-extension <extension>
```

where _extension_ is either an extension identifier (ie, `redhat.java`) or the full path to a `vsix` file on the local disk.

> __NOTE:__ Currently, `oni2` is only added to the user's `PATH` on Windows. Find instructions on adding it to your `PATH` on macOS and Linux over [here](./../using-onivim/command-line.md).

### Publishing an Extension

Open VSX is still new - so there are many extensions that aren't hosted there, yet. 

If your favorite extension is missing - you can help us out by publishing it to Open VSX:

1) Register for an account using the [Open VSX GitHub OAuth](https://open-vsx.org/oauth2/authorization/github) provider
2) Create a [personal access token](https://open-vsx.org/user-settings/tokens) 
3) Install the `ovsx` tool - `npm install -g ovsx`
4) Create a namespace corrresponding to your extension: `ovsx create-namespace <publisher> --pat <token>`
5) Run `ovsx publish --pat <token>` in the directory of the extension you want to publish.

## Vim Extensions

WIP

## Listing Extensions via the UI

Installed extensions can be viewed in the extensions pane:

![extensions-pane](https://user-images.githubusercontent.com/13532591/88988024-911be400-d28c-11ea-91db-4decbeb37eb8.png)

or, via the CLI: `oni2 --list-extensions` command.
