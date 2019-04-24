/*
 * State.re
 *
 * Top-level state of the editor
 */

open Oni_Core;
open Oni_Core.Types;

type t = {
  mode: Mode.t,
  diagnostics: Diagnostics.t,
  buffers: Buffers.t,
  editorFont: EditorFont.t,
  uiFont: UiFont.t,
  menu: Menu.t,
  commandline: Commandline.t,
  wildmenu: Wildmenu.t,
  configuration: Configuration.t,
  syntaxHighlighting: SyntaxHighlighting.t,
  theme: Theme.t,
  editorGroups: EditorGroups.t,
  inputControlMode: Input.controlMode,
  iconTheme: IconTheme.t,
  languageInfo: LanguageInfo.t,
  lifecycle: Lifecycle.t,
  statusBar: StatusBarModel.t,
  editorLayout: WindowManager.t,
};

let create: unit => t =
  () => {
    configuration: Configuration.default,
    diagnostics: Diagnostics.create(),
    mode: Insert,
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
    syntaxHighlighting: SyntaxHighlighting.create(),
    theme: Theme.create(),
    editorGroups: EditorGroups.create(),
    inputControlMode: EditorTextFocus,
    iconTheme: IconTheme.create(),
    languageInfo: LanguageInfo.create(),
    statusBar: StatusBarModel.create(),
    editorLayout: WindowManager.create(),
  };
