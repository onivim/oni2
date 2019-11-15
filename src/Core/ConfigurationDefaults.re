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
        { "key": "<C-TAB>", "command": "workbench.action.openNextRecentlyUsedEditorInGroup", "when": [["editorTextFocus"]] },
        { "key": "<C-P>", "command": "workbench.action.quickOpen", "when": [["editorTextFocus"]] },
        { "key": "<D-P>", "command": "workbench.action.quickOpen", "when": [["editorTextFocus"]] },
        { "key": "<S-C-P>", "command": "workbench.action.showCommands", "when": [["editorTextFocus"]] },
        { "key": "<D-S-P>", "command": "workbench.action.showCommands", "when": [["editorTextFocus"]] },
        { "key": "<C-V>", "command": "editor.action.clipboardPasteAction", "when": [["insertMode"]] },
        { "key": "<D-V>", "command": "editor.action.clipboardPasteAction", "when": [["insertMode"]] },
        { "key": "<ESC>", "command": "workbench.action.closeQuickOpen", "when": [["menuFocus"]] },
        { "key": "<C-N>", "command": "list.focusDown", "when": [["menuFocus"], ["textInputFocus"]] },
        { "key": "<C-P>", "command": "list.focusUp", "when": [["menuFocus"], ["textInputFocus"]]},
        { "key": "<D-N>", "command": "list.focusDown", "when": [["menuFocus"], ["textInputFocus"]] },
        { "key": "<D-P>", "command": "list.focusUp", "when": [["menuFocus"], ["textInputFocus"]] },
        { "key": "<TAB>", "command": "list.focusDown", "when": [["menuFocus"], ["textInputFocus"]] },
        { "key": "<S-TAB>", "command": "list.focusUp", "when": [["menuFocus"], ["textInputFocus"]] },
        { "key": "<CR>", "command": "list.select", "when": [["menuFocus"], ["textInputFocus"]] },
        { "key": "<S-C-B>", "command": "explorer.toggle", "when": [["editorTextFocus"]]},
        { "key": "<C-P>", "command": "selectPrevSuggestion", "when": [["suggestWidgetVisible"]] },
        { "key": "<C-N>", "command": "selectNextSuggestion", "when": [["suggestWidgetVisible"]] },
        { "key": "<CR>", "command": "insertBestCompletion", "when": [["suggestWidgetVisible"]] },
        { "key": "<TAB>", "command": "insertBestCompletion", "when": [["suggestWidgetVisible"]] }
    ]
}
|},
    )
  | _ => None
  };
