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
  tabs: list(Tab.t),
  buffers: BufferMap.t,
  activeBufferId: int,
  editorFont: EditorFont.t,
  uiFont: UiFont.t,
  menu: UiMenu.t,
  commandline: Commandline.t,
  wildmenu: Wildmenu.t,
  configuration: Configuration.t,
  syntaxHighlighting: SyntaxHighlighting.t,
  theme: Theme.t,
  editor: Editor.t,
  inputControlMode: Input.controlMode,
  selection: VisualRange.t,
};

let create: unit => t =
  () => {
    configuration: Configuration.create(),
    mode: Insert,
    menu: Menu.create(),
    commandline: Commandline.create(),
    wildmenu: Wildmenu.create(),
    activeBufferId: 0,
    buffers: BufferMap.Buffers.add(0, Buffer.ofLines([||]), BufferMap.empty),
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
    tabs: [Tab.create(0, "[No Name]")],
    theme: Theme.create(),
    editor: Editor.create(),
    inputControlMode: EditorTextFocus,
    selection: VisualRange.create(),
  };
