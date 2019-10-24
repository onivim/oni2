---
id: installation
title: Installation
sidebar_label: Installation
---

While we're developing Onivim 2, the latest builds are available at our [early access portal](https://v2.onivim.io/early-access-portal).

## System Requirements

### OSX

- Requires OSX 10.13+

> NOTE: We're [working on 10.12 support](https://github.com/onivim/oni2/issues/902), but it is not currently available.

### Windows

- x64 Only
- Windows 8+ or higher

> NOTE: Windows 7 will also be supported, but there is a [blocking issue](https://github.com/onivim/oni2/issues/850)

### Linux 

- x64 Only
- `glibc` 2.17+

Tested on:
- Ubuntu 16.04 +
- CentOS 7+
- Manjaro

## Guide

### OSX (10.13+)

1. Download `Onivim2` for macOS.
2. Double-click on the downloaded DMG to expand the contents.
3. Drag `Onivim2` to the `Applications` Folder
4. Open `Finder` (`Command+Space`) and type `Onivim2`, press `Enter`.

> __NOTE:__ A gatekeeper dialog will open on the first run - press 'Open' to start the application.

5. _OPTIONAL:_ Add Onivim2 to your Dock by right-clicking on the icon to bring up the context menu, and choosing __Options, Keep in Dock.__

### Windows (x64)

1. Download the Onivim 2 Installer for Windows.
2. Once it is downloaded, run the installer (Onivim2.exe).

> __NOTE:__ Even though the app is code-signed, SmartScreen may show up as the app builds up 'reputation'. Microsoft establishes the reputation of an executable based on the number of installations; so we need to build up some 'reputation'. 
>
> Validate that the publisher is listed as "Outrun Labs, LLC" and you can proceed with the installation.

3. By default, the setup program installs Onivim 2 in `C:\Program Files\Onivim2\Oni2.exe`.

### Linux (x64)

1. Download `Onivim2.AppImage`.
2. `$ chmod u+x Onivim2.AppImage`.
3. `$ ./Onivim2.AppImage`.
