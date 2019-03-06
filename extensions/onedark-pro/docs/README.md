# One Dark Pro

## About
[One Dark Pro](https://marketplace.visualstudio.com/items?itemName=zhuangtongfa.Material-theme) is based on Atom's default One Dark theme, and is one of the most downloaded themes for VS Code. This document will show you how to install the theme on VS Code, and how to develop and contribute to this project.
![screenshot](https://ws1.sinaimg.cn/large/006tNbRwgy1fvwjzw3c8fj31kw101adb.jpg)


# screenshot
![Screenshot](https://ws3.sinaimg.cn/large/006tNbRwgy1fvwkr6i199j31kw16otat.jpg)

![Screenshot](https://ws2.sinaimg.cn/large/006tNbRwgy1fvwkrv2rorj31kw16odhw.jpg)

## Install
Using the Extensions menu, search for **'One Dark Pro'**. Don't forget to apply the theme (see below).

![ScreenShot](https://ws2.sinaimg.cn/large/006tNbRwgy1fvwijszewzj31kw101doj.jpg)

## Apply
Press `ctrl(⌘) + k`, then press `ctrl(⌘) + t`, you will see a theme selection interface. Choose **'One Dark Pro'**.

![ScreenShot](https://ws1.sinaimg.cn/large/006tNbRwgy1fvwilva97dj31kw101k08.jpg)

## Develop 
If you see any inconsistencies or missing colors, the following guide will show you how to make your own changes. You can submit your improvements as a merge request to this theme.

### Find the VS Code extension path

In a terminal, `cd` to your themes folder

- Windows:
`C:\Users\yourUserName\.vscode\extensions\themes`  

- Mac/linux:
`~/.vscode/extensions/themes`

![ScreenShot](https://ws4.sinaimg.cn/large/006tNbRwgy1fvwin60gwrj31kw14dwem.jpg)

Then open 'color.json', 'syntax.json', 'editor.json' with VS Code: `code src`

![ScreenShot](https://ws2.sinaimg.cn/large/006tNbRwgy1fvwioznq0dj31kw101tfx.jpg)

### Example from 'syntax.json'
The following is a code snippet taken from the **'syntax.json'** file:

![ScreenShot](https://ws3.sinaimg.cn/large/006tNbRwgy1fvwiqzief1j30z80qkwf3.jpg)

### Principle
VS Code will parse code and specify a scope for each piece of syntax. For example, the scope may be a keyword, a constant, or punctuation. **'syntax.json'** includes the settings that tell VS Code how to format the text accordingly, using these scopes.

### Common scope list

```
comment
constant
constant.character.escape
constant.language
constant.numeric
declaration.section entity.name.section
declaration.tag
deco.folding
entity.name.function
entity.name.tag
entity.name.type
entity.other.attribute-name
entity.other.inherited-class
invalid
invalid.deprecated.trailing-whitespace
keyword
keyword.control.import
keyword.operator.js
markup.heading
markup.list
markup.quote
meta.embedded
meta.preprocessor
meta.section entity.name.section
meta.tag
storage
storage.type.method
string
string source
string.unquoted
support.class
support.constant
support.function
support.type
support.variable
text source
variable
variable.language
variable.other
variable.parameter
```

### Get code scope
VS Code comes with a built-in tool to easily obtain the scope of a piece of syntax. 

Press `ctrl(⌘) + shift + P`, then type `dev`, and choose **"Developer: Inspect TM Scopes"** option.

This will show you the selected token's scope. There are four sections:

- the in-scope piece of syntax

- language, token type, etc.

- the theme rule and shows the foreground color of the token

- the list of scopes for the token

![ScreenShot](https://ws1.sinaimg.cn/large/006tNbRwgy1fvwjl9f2igj31kw101jxc.jpg)
![ScreenShot](https://ws2.sinaimg.cn/large/006tNbRwgy1fvwjlnqha0j31kw1017a3.jpg)

### Add/Change code color
Now you know the rules for the theme, you simply need the code scope and the hex color you would like. Now edit the **'syntax.json'** file, add/change code snippet like this:

```js
{
      "name": "c++ function",
      "scope": "meta.function.c",
      "settings": {
        "foreground":  colorObj['coral']
      }
}
```

then run `node build.js` in your terminal (Notice: need Node.JS 6.0+, or just change the 'OneDark-Pro.json' then `reload`(Not recommended))



### Reload
Then press `ctrl(⌘) + shift + P`, type **'reload'** and press `enter`. Once the window has reloaded, you will find the color of the code has changed.

![screenshot](https://ws3.sinaimg.cn/large/006tNbRwgy1fvwjm9anuij31kw101aft.jpg)

### Colors config

`color.json`
![screenshot](https://ws3.sinaimg.cn/large/006tNbRwgy1fvwjxs1cc0j31kw101n1h.jpg)

```json
{
  "classic": {
    "purple": "#c678dd",
    "error": "#f44747",
    "coral": "#e06c75",
    "whiskey": "#d19a66",
    "chalky": "#e5c07b",
    "lightDark": "#7f848e",
    "dark": "#5c6370",
    "malibu": "#61afef",
    "green": "#98c379",
    "fountainBlue": "#56b6c2",
    "invalid": "#ffffff"
  },
  "vivid": {
    "purple": "#d55fde",
    "error": "#f44747",
    "coral": "#ef596f",
    "whiskey": "#d19a66",
    "chalky": "#e5c07b",
    "lightDark": "#7f848e",
    "dark": "#5c6370",
    "malibu": "#61afef",
    "green": "#89ca78",
    "fountainBlue": "#2bbac5",
    "invalid": "#ffffff"
  }
}


```

## Workbench theming
If you want to play around with new colors, use the setting `workbench.colorCustomizations` to customize the currently selected theme.
For example, you can add this snippet in your "settings.json" file:

```json
"workbench.colorCustomizations":{
    "tab.activeBackground": "#282c34",
    "activityBar.background": "#282c34",
    "sideBar.background": "#282c34"
}
```

Please check the official documentation, [Theme Color Reference](https://code.visualstudio.com/docs/getstarted/theme-color-reference), for more helpful information.

## User definable syntax highlighting colors
You also can custom your syntax highlighting in "setting.json"

![setting.json](https://ws4.sinaimg.cn/large/006tNbRwgy1fvwjoqnbtgj31kw101whv.jpg)

![custom](https://ws3.sinaimg.cn/large/006tNbRwgy1fvwjpwnq7bj30qu14w3zr.jpg)


[More info](https://code.visualstudio.com/updates/v1_15#_user-definable-syntax-highlighting-colors)

## Contribute
Now you know how to develop the theme, you can fork this repository and send a pull request with your version. The request will be reviewed, and if successful, merged into this theme and published on the VS Code market.

To publish your own theme, please refer to the official documentation: [https://code.visualstudio.com/docs/extensions/themes-snippets-colorizers](https://code.visualstudio.com/docs/extensions/themes-snippets-colorizers)  
