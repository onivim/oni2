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
};

let create: unit => t =
  () => {
    mode: Insert,
    buffer: Buffer.ofLines([|"testing"|]),
    editorFont:
      EditorFont.create(
        ~fontFile="FiraCode-Regular.ttf",
        ~fontSize=14,
        ~measuredWidth=0,
        ~measuredHeight=0,
        (),
      ),
    tabs: [Tab.create(0, "[No Name]")],
  };
