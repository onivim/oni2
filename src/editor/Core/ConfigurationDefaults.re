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
  "editor.minimap.enabled": true,
  "editor.insertSpaces": false,
  "editor.indentSize": 4,
  "editor.tabSize": 4,
  "editor.DetectIndentation": true,
  "editor.LargeFileOptimizations": true,
  "editor.matchBrackets": true,
  "editor.minimap.enabled": true,
  "editor.minimap.showSlider": true,
  "editor.minimap.maxColumn": 120,
  "editor.lineNumbers": "on",
  "editor.insertSpaces": false,
  "editor.indentSize": 4,
  "editor.tabSize": 4,
  "editor.renderIndentGuides": true,
  "editor.highlightActiveIndentGuide": true,
  "editor.renderWhitespace": "all",
  "editor.rulers": [],
  "workbench.activityBar.visible": true,
  "workbench.editor.showTabs": true,
  "workbench.sideBar.visible": true,
  "workbench.statusBar.visible": true,
  "workbench.iconTheme": "vs-seti",
  "files.exclude": ["_esy", "node_modules"],
  "editor.zenMode.hideTabs": true
}
|},
    )
  | "keybindings.json" =>
    Some(
      {|
{
    "bindings": [
        { "key": "<C-P>", "command": "quickOpen.open", "when": [["editorTextFocus"]] },
        { "key": "<C-V>", "command": "editor.action.clipboardPasteAction", "when": [["editorTextFocus"]] },
        { "key": "<D-V>", "command": "editor.action.clipboardPasteAction", "when": [["editorTextFocus"]] },
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
