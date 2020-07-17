---
id: python
title: Python
sidebar_label: Python
---

![python-support](https://user-images.githubusercontent.com/13532591/87734981-f9e16780-c788-11ea-864e-eadef5958a70.png)

Onivim supports several features from Microsoft's `vscode-python` extension, including:

- Code completion
- Go-to-definition
- Hover
- Signature Help

## Setup 

> NOTE: There is a blocking bug (https://github.com/microsoft/vscode-python/issues/12465) in the latest open-vsx package, so the extension
needs to be installed manually.

1) Download the extension: https://github.com/microsoft/vscode-python/releases/download/2020.5.86806/ms-python-release.vsix
2) Run `oni2 --install-extension /path/to/ms-python-release.vsix`
3) Activate your virtual environment in your python project
4) Open `oni2` from your python project folder

If all goes well - you'll see the current python interpreter in the status bar:

![python-status-bar](https://user-images.githubusercontent.com/13532591/87735005-141b4580-c789-11ea-9c68-a3583cd4b397.png)

## FAQ

### I'm not getting completions or diagnostics - what can I try?

1) Make sure you start Onivim from the active virtual environment. This will set the path to the python modules for the vscode-python extension.
2) If the Python interpreter is not inferred correctly, you can set the `"python.pythonPath": "/path/to/python"` configuration setting manually.
3) If the modules your project using are in a non-standard location, you can add them via the `"python.autoComplete.extraPaths": ["/path/to/customModule"]` configuration setting.
