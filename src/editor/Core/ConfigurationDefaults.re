/*
 * ConfigurationDefaults.re
 *
 * Configuration defaults in string form to generate default configuration from.
 */

let getDefaultConfigString = configName =>
  switch (configName) {
  | "configuration.json" =>
    Some(
      {|
{
  "editor.detectIndentation": true,
  "editor.fontFamily": "FiraCode-Regular.ttf",
  "editor.fontSize": 14,
  "editor.largeFileOptimizations": true,
  "editor.highlightActiveIndentGuide": true,
  "editor.indentSize": 4,
  "editor.insertSpaces": false,
  "editor.lineNumbers": "on",
  "editor.matchBrackets": true,
  "editor.minimap.enabled": true,
  "editor.minimap.maxColumn": 120,
  "editor.minimap.showSlider": true,
  "editor.renderIndentGuides": true,
  "editor.renderWhitespace": "all",
  "editor.rulers": [],
  "editor.tabSize": 4,
  "editor.zenMode.hideTabs": true,
  "editor.zenMode.singleFile": true,
  "files.exclude": ["_esy", "node_modules"],
  "workbench.activityBar.visible": true,
  "workbench.editor.showTabs": true,
  "workbench.iconTheme": "vs-seti",
  "workbench.sideBar.visible": true,
  "workbench.statusBar.visible": true,
  "workbench.tree.indent": 2,
  "vim.useSystemClipboard": ["yank"]
}
|},
    )
  | "keybindings.json" =>
    Some(
      {|
{
    "bindings": [
        { "key": "<C-P>", "command": "quickOpen.open", "when": [["editorTextFocus"]] },
        { "key": "<C-TAB>", "command": "quickOpen.openFiles", "when": [["editorTextFocus"]] },
        { "key": "<C-V>", "command": "editor.action.clipboardPasteAction", "when": [["insertMode"]] },
        { "key": "<D-V>", "command": "editor.action.clipboardPasteAction", "when": [["insertMode"]] },
        { "key": "<D-P>", "command": "quickOpen.open", "when": [["editorTextFocus"]] },
        { "key": "<S-C-P>", "command": "commandPalette.open", "when": [["editorTextFocus"]] },
        { "key": "<D-S-P>", "command": "commandPalette.open", "when": [["editorTextFocus"]] },
        { "key": "<ESC>", "command": "menu.close", "when": [["menuFocus"]] },
        { "key": "<C-N>", "command": "menu.next", "when": [["menuFocus"], ["textInputFocus"]] },
        { "key": "<C-P>", "command": "menu.previous", "when": [["menuFocus"], ["textInputFocus"]]},
        { "key": "<D-N>", "command": "menu.next", "when": [["menuFocus"], ["textInputFocus"]] },
        { "key": "<D-P>", "command": "menu.previous", "when": [["menuFocus"], ["textInputFocus"]] },
        { "key": "<C-N>", "command": "wildmenu.next", "when": [["commandLineFocus"]] },
        { "key": "<C-P>", "command": "wildmenu.previous", "when": [["commandLineFocus"]] },
        { "key": "<D-N>", "command": "wildmenu.next", "when": [["commandLineFocus"]] },
        { "key": "<D-P>", "command": "wildmenu.previous", "when": [["commandLineFocus"]] },
        { "key": "<TAB>", "command": "wildmenu.next", "when": [["commandLineFocus"]] },
        { "key": "<S-TAB>", "command": "wildmenu.previous", "when": [["commandLineFocus"]] },
        { "key": "<CR>", "command": "menu.select", "when": [["menuFocus"], ["textInputFocus"]] },
        { "key": "<S-C-B>", "command": "explorer.toggle", "when": [["editorTextFocus"]]}
    ]
}
|},
    )
  | _ => None
  };
