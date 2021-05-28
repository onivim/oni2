---
id: formatting
title: Formatting
sidebar_label: Formatting
---

## Formatting

![format-demo](https://user-images.githubusercontent.com/13532591/111513762-5dec6f00-870e-11eb-9db3-f8ab64a621f6.gif)

Onivim supports code formatting, powered by Code language extensions. Onivim comes bundled with several format providers by default.

### Keybindings

Onivim supports both the `=` and `gq` operators from Vim - in Onivim, these are functionality equivalent, and can be used to format a range of text.

In both cases, Onivim requests from an installed extension that provides format capabilities.

For example:

![gqap](https://user-images.githubusercontent.com/13532591/111513758-5cbb4200-870e-11eb-86ac-f98de185aff2.gif)

In the above example, the `gqap` key sequence does the following:
- `gq` format operator
- `ap` around-paragraph motion

In addition, the document can be formatted using these keybindings:

- Linux: <kbd>Control</kbd>+<kbd>Shift</kbd>+<kbd>I</kbd> 
- Mac: <kbd>Option</kbd>+<kbd>Shift</kbd>+<kbd>F</kbd> 
- Windows: <kbd>Alt</kbd>+<kbd>Shift</kbd>+<kbd>F</kbd> 

### Multiple Format Providers

If there are multiple available format providers for the active filetype, Onivim will prompt to choose the default format provider:

![format-provider](https://user-images.githubusercontent.com/13532591/111514555-161a1780-870f-11eb-9bdf-dc1c48852866.png)

### Configuration

#### Format on Save

The `"editor.formatOnSave"` setting can be used to request that Onivim runs a formatter after saving:

![format-on-save](https://user-images.githubusercontent.com/13532591/111513965-85433c00-870e-11eb-8cfc-423a7c4d71de.gif)

This setting is recommended to be set per-filetype, for example:

```
"[javascript]": {
    "editor.formatOnSave": true
}
```
