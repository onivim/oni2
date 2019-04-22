/*
 * State.re
 *
 * Top-level state of the editor
 */

open Oni_Core;
open Oni_Core.Types;

module Tab = {
  type t = {
    id: int,
    title: string,
    active: bool,
    modified: bool,
  };

  let create = (id, title) => {id, title, active: false, modified: false};
};

type t = {
  mode: Mode.t,
  diagnostics: Diagnostics.t,
  tabs: list(Tab.t),
  buffers: BufferMap.t,
  editorFont: EditorFont.t,
  uiFont: UiFont.t,
  menu: Menu.t,
  commandline: Commandline.t,
  wildmenu: Wildmenu.t,
  configuration: Configuration.t,
  syntaxHighlighting: SyntaxHighlighting.t,
  theme: Theme.t,
  editor: Editor.t,
  editors: IntMap.t(Editor.t),
  activeEditorId: int,
  inputControlMode: Input.controlMode,
  iconTheme: IconTheme.t,
  languageInfo: LanguageInfo.t,
  statusBar: StatusBarModel.t,
  editorLayout: WindowManager.t,
};

let defaultEditor = Editor.create();
let editors = IntMap.empty |> IntMap.add(defaultEditor.id, defaultEditor);

let create: unit => t =
  () => {
    configuration: Configuration.default,
    diagnostics: Diagnostics.create(),
    mode: Insert,
    menu: Menu.create(),
    commandline: Commandline.create(),
    wildmenu: Wildmenu.create(),
    activeBufferId: 0,
    buffers: BufferMap.empty,
    editorFont:
      EditorFont.create(
        ~fontFile="FiraCode-Regular.ttf",
        ~fontSize=14,
        ~measuredWidth=1.,
        ~measuredHeight=1.,
        (),
      ),
    uiFont: UiFont.create(~fontFile="selawk.ttf", ~fontSize=12, ()),
    syntaxHighlighting: SyntaxHighlighting.create(),
    tabs: [],
    theme: Theme.create(),
    activeEditorId: defaultEditor.id,
    editors,
    inputControlMode: EditorTextFocus,
    iconTheme: IconTheme.create(),
    languageInfo: LanguageInfo.create(),
    statusBar: StatusBarModel.create(),
    editorLayout: WindowManager.create(),
  };
