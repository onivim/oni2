/*
 * ConfigurationValues.re
 *
 * Configuration settings for the editor
 */

[@deriving show({with_path: false})]
type editorRenderWhitespace =
  | All
  | Boundary
  | None;

[@deriving show({with_path: false})]
type t = {
  editorDetectIndentation: bool,
  editorLargeFileOptimizations: bool,
  editorLineNumbers: LineNumber.setting,
  editorMatchBrackets: bool,
  editorMinimapEnabled: bool,
  editorMinimapShowSlider: bool,
  editorMinimapMaxColumn: int,
  editorInsertSpaces: bool,
  editorIndentSize: int,
  editorTabSize: int,
  editorHighlightActiveIndentGuide: bool,
  editorRenderIndentGuides: bool,
  editorRenderWhitespace,
  workbenchActivityBarVisible: bool,
  /* Onivim2 specific setting */
  workbenchSideBarVisible: bool,
  workbenchEditorShowTabs: bool,
  workbenchStatusBarVisible: bool,
  workbenchIconTheme: string,
  filesExclude: list(string),
  zenModeHideTabs: bool,
};

let default = {
  editorDetectIndentation: true,
  editorLargeFileOptimizations: true,
  editorMatchBrackets: true,
  editorMinimapEnabled: true,
  editorMinimapShowSlider: true,
  editorMinimapMaxColumn: Constants.default.minimapMaxColumn,
  editorLineNumbers: On,
  editorInsertSpaces: false,
  editorIndentSize: 4,
  editorTabSize: 4,
  editorRenderIndentGuides: true,
  editorHighlightActiveIndentGuide: true,
  editorRenderWhitespace: All,
  workbenchActivityBarVisible: true,
  workbenchEditorShowTabs: true,
  workbenchSideBarVisible: true,
  workbenchStatusBarVisible: true,
  workbenchIconTheme: "vs-seti",
  filesExclude: ["node_modules", "_esy"],
  zenModeHideTabs: true,
};

let getDefaultConfigString = configName =>
  switch (configName) {
  | "configuration.json" =>
    Some(
      {|
{
  "editor.minimap.enabled": true,
  "editor.insertSpaces": false,
  "editor.indentSize": 4,
  "editor.tabSize": 4
}
|},
    )
  | "keybindings.json" =>
    Some(
      {|
{
    "bindings": [
        { "key": "<C-P>", "command": "quickOpen.open", "when": [["editorTextFocus"]] },
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
