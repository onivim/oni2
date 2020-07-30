---
id: publishing
title: Publishing Extensions
sidebar_label: Publishing Extensions
---

## VSCode Extensions

The VSCode marketplace is unfortunately proprietary - so we use the [Open VSX](https://open-vsx.org) marketplace, which is open, vendor-neutral, and free to use.

There are more details about the Open VSX project in the [Eclipse Open VSX article](https://www.eclipse.org/community/eclipse_newsletter/2020/march/1.php)

### Publishing

Publishing extensions to [Open VSX](https://open-vsx.org) is easy:

1) Register for an account using the [Open VSX GitHub OAuth](https://open-vsx.org/oauth2/authorization/github) provider
2) Create a [personal access token](https://open-vsx.org/user-settings/tokens) 
3) Install the `ovsx` tool - `npm install -g ovsx`
4) Create a namespace corrresponding to your extension: `ovsx create-namespace <publisher> --pat <token>`
5) Run `ovsx publish --pat <token>` in the directory of the extension you want to publish.
