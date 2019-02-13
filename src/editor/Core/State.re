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
  buffer: Buffer.t,
  editorFont: EditorFont.t,
  cursorPosition: BufferPosition.t,
  commandline: Commandline.t,
  wildmenu: Wildmenu.t,
  configuration: Configuration.t,
  theme: Theme.t,
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
    buffer: Buffer.ofLines([||]),
    cursorPosition: BufferPosition.createFromZeroBasedIndices(0, 0),
    editorFont:
      EditorFont.create(
        ~fontFile="FiraCode-Regular.ttf",
        ~fontSize=14,
        ~measuredWidth=0,
        ~measuredHeight=0,
        (),
      ),
    tabs: [Tab.create(0, "[No Name]")],
    theme: Theme.create(),
  };
