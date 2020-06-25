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
  "editor.fontFamily": "JetBrainsMono-Regular.ttf",
  "editor.fontSize": 14,
  "editor.largeFileOptimizations": true,
  "editor.highlightActiveIndentGuide": true,
  "editor.indentSize": 4,
  "editor.insertSpaces": true,
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
  "files.exclude": ["_esy", "node_modules", ".git"],
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
[
  // See the onivim documentation for details on the format:
  // https://onivim.github.io/docs/configuration/key-bindings
  // Add key bindings here, for example:
  // { "key": "<TAB>", "command": "workbench.action.quickOpen", when: "editorTextFocus" },
  // { "key": "jk", "command": "vim.esc", when: "insertMode" },
]
|},
    )
  | _ => None
  };
