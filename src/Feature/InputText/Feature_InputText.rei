open Oni_Core;

[@deriving show]
type msg;

[@deriving show]
type model;

let value: model => string;

let create: (~placeholder: string) => model;

let empty: model;

let update: (msg, model) => model;

let handleInput: (~key: string, model) => model;

let paste: (~text: string, model) => model;

let set: (~text: string, ~cursor: int, model) => model;
let setPlaceholder: (~placeholder: string, model) => model;

let isCursorAtEnd: model => bool;

let cursorPosition: model => int;

let isEmpty: model => bool;

module View: {
  let make:
    (
      ~key: Brisk_reconciler.Key.t=?,
      ~model: model,
      ~theme: ColorTheme.Colors.t,
      ~style: list(Revery.UI.Style.allProps)=?,
      ~fontSize: float=?,
      ~fontFamily: Revery.Font.Family.t=?,
      ~placeholderColor: Revery.Color.t=?,
      ~cursorColor: Revery.Color.t=?,
      ~selectionColor: Revery.Color.t=?,
      ~prefix: string=?,
      ~isFocused: bool,
      ~dispatch: msg => unit,
      unit
    ) =>
    Revery.UI.element;
};
