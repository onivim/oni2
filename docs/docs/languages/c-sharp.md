---
id: c-sharp
title: C#
sidebar_label: C#
---

Onivim supports C# via the [`muhammad-sammy.csharp`](https://open-vsx.org/extension/muhammad-sammy/csharp) extension:

![c-sharp](https://user-images.githubusercontent.com/13532591/89577746-fcfad100-d7e5-11ea-9e96-afb1edf630e2.png)

The following features are supported:
- Code completion
- Diagnostics
- Go-to definition
- Signature Help

### Setup

1) Ensure you have the latest version of the .NET Core tools: https://docs.microsoft.com/en-us/dotnet/core/install/

2) Install the extension via the command-line:

- `oni2 --install-extension muhammad-sammy.csharp`

### Usage

Once installed, run `oni2` in the your project's folder - for example: `oni2 /path/to/csharp-project`, and then open a C# file.

The first time the extension activates, it will automatically download and install the OmniSharp server.

After the extension installs required dependencies, you'll see a status bar icon showing the server status, as well as the current project:

![status-bar](https://user-images.githubusercontent.com/13532591/89578393-ea34cc00-d7e6-11ea-8b66-7b2f15151dfb.png)
