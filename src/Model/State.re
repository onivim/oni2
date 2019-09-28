/*
 * State.re
 *
 * Top-level state of the editor
 */

open Oni_Core;
open Oni_Core.Types;
open Oni_Syntax;

type t = {
  mode: Vim.Mode.t,
  diagnostics: Diagnostics.t,
  buffers: Buffers.t,
  editorFont: EditorFont.t,
  uiFont: UiFont.t,
  menu: Menu.t,
  commandline: Commandline.t,
  wildmenu: Wildmenu.t,
  configuration: Configuration.t,
  // New-school native syntax highlighting
  syntaxHighlighting: SyntaxHighlighting.t,
  // Theme is the UI shell theming
  theme: Theme.t,
  // Token theme is theming for syntax highlights
  tokenTheme: TokenTheme.t,
  editorGroups: EditorGroups.t,
  iconTheme: IconTheme.t,
  keyBindings: KeyBindings.t,
  keyDisplayer: KeyDisplayer.t,
  languageInfo: LanguageInfo.t,
  lifecycle: Lifecycle.t,
  notifications: Notifications.t,
  searchHighlights: SearchHighlights.t,
  statusBar: StatusBarModel.t,
  windowManager: WindowManager.t,
  fileExplorer: FileExplorer.t,
  zenMode: bool,
};

let create: unit => t =
  () => {
    configuration: Configuration.default,
    diagnostics: Diagnostics.create(),
    mode: Normal,
    menu: Menu.create(),
    commandline: Commandline.create(),
    wildmenu: Wildmenu.create(),
    buffers: Buffers.empty,
    editorFont:
      EditorFont.create(
        ~fontFile="FiraCode-Regular.ttf",
        ~fontSize=14,
        ~measuredWidth=1.,
        ~measuredHeight=1.,
        (),
      ),
    lifecycle: Lifecycle.create(),
    uiFont: UiFont.create(~fontFile="selawk.ttf", ~fontSize=12, ()),
    syntaxHighlighting: SyntaxHighlighting.empty,
    theme: Theme.default,
    tokenTheme: TokenTheme.empty,
    editorGroups: EditorGroups.create(),
    iconTheme: IconTheme.create(),
    keyBindings: KeyBindings.default,
    keyDisplayer: KeyDisplayer.empty,
    languageInfo: LanguageInfo.create(),
    notifications: Notifications.default,
    searchHighlights: SearchHighlights.create(),
    statusBar: StatusBarModel.create(),
    windowManager: WindowManager.create(),
    fileExplorer: FileExplorer.create(),
    zenMode: false,
  };
