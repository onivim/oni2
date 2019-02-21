/*
 * State.re
 *
 * Top-level state of the editor
 */

open Types;

module Tab = {
  type t = {
    id: int,
    title: string,
    active: bool,
  };

  let create = (id, title) => {id, title, active: false};
};

type t = {
  mode: Mode.t,
  tabs: list(Tab.t),
  buffers: BufferMap.t,
  activeBufferId: int,
  editorFont: EditorFont.t,
  cursorPosition: BufferPosition.t,
  commandline: Commandline.t,
  wildmenu: Wildmenu.t,
  configuration: Configuration.t,
  theme: Theme.t,
  editorView: EditorView.t,
};

let create: unit => t =
  () => {
    configuration: Configuration.create(),
    mode: Insert,
    commandline: {
      content: "",
      firstC: "",
      prompt: "",
      position: 0,
      indent: 0,
      level: 0,
      show: false,
    },
    wildmenu: {
      items: [],
      selected: 0,
      show: false,
    },
    activeBufferId: 0,
    buffers: BufferMap.Buffers.add(0, Buffer.ofLines([||]), BufferMap.empty),
    cursorPosition: BufferPosition.createFromZeroBasedIndices(0, 0),
    editorFont:
      EditorFont.create(
        ~fontFile="FiraCode-Regular.ttf",
        ~fontSize=14,
        ~measuredWidth=1,
        ~measuredHeight=1,
        (),
      ),
    tabs: [Tab.create(0, "[No Name]")],
    theme: Theme.create(),
    editorView: EditorView.create(~scrollY=0, ()),
  };
