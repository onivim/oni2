open Oni_Core;

module Editor = {
  let detectIndentation = "editor.detectIndentation";
  let fontFamily = "editor.fontFamily";
  let fontSize = "editor.fontSize";
  let largeFileOptimization = "editor.largeFileOptimizations";
  let highlightActiveIndentGuide = "editor.highlightActiveIndentGuide";
  let indentSize = "editor.indentSize";
  let insertSpaces = "editor.insertSpaces";
  let lineNumbers = "editor.lineNumbers";
  let matchBrackets = "editor.matchBrackets";
  let renderIndentGuides = "editor.renderIndentGuides";
  let renderWhitespace = "editor.renderWhitespace";
  let rulers = "editor.rulers";
  let tabSize = "editor.tabSize";

  module Minimap = {
    let enabled = "editor.minimap.enabled";
    let maxColumn = "editor.minimap.maxColumn";
    let showSlider = "editor.minimap.showSlider";
  };

  module ZenMode = {
    let hideTabs = "editor.zenMode.hideTabs";
    let singleFile = "editor.zenMode.singleFile";
  };

  let defaults =
    Json.Encode.[
      (detectIndentation, bool(true)),
      (fontFamily, string("FiraCode-Regular.ttf")),
      (fontSize, int(14)),
      (largeFileOptimization, bool(true)),
      (highlightActiveIndentGuide, bool(true)),
      (indentSize, int(4)),
      (insertSpaces, bool(false)),
      (lineNumbers, string("one")),
      (matchBrackets, bool(true)),
      (renderIndentGuides, bool(true)),
      (renderWhitespace, string("all")),
      (rulers, list(int, [])),
      (tabSize, int(4)),
      (Minimap.enabled, bool(true)),
      (Minimap.maxColumn, int(120)),
      (Minimap.showSlider, bool(true)),
      (ZenMode.hideTabs, bool(true)),
      (ZenMode.singleFile, bool(true)),
    ];
};

module Experimental = {};

module Files = {
  let exclude = "files.exclude";

  let defaults =
    Json.Encode.[(exclude, list(string, ["_esy", "node_modules"]))];
};

module Vim = {
  let useSystemClipboard = "vim.useSystemClipboard";

  let defaults =
    Json.Encode.[(useSystemClipboard, list(string, ["yank"]))];
};

module Workbench = {
  let iconTheme = "workbench.iconTheme";

  module ActivityBar = {
    let visible = "workbench.activityBar.visible";
  };

  module Editor = {
    let showTabs = "workbench.editor.showTabs";
  };

  module SideBar = {
    let visible = "workbench.sideBar.visible";
  };

  module StatusBar = {
    let visible = "workbench.statusBar.visible";
  };

  module Tree = {
    let indent = "workbench.tree.indent";
  };

  let defaults =
    Json.Encode.[
      (iconTheme, string("vs-seti")),
      (ActivityBar.visible, bool(true)),
      (Editor.showTabs, bool(true)),
      (SideBar.visible, bool(true)),
      (StatusBar.visible, bool(true)),
      (Tree.indent, int(2)),
    ];
};
