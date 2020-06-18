open Oni_Core;

// MODEL

type panel =
  | Left
  | Center
  | Bottom;

type model;

let initial: model;

let windows: model => list(int);
let addWindow: ([ | `Horizontal | `Vertical], int, model) => model;
let insertWindow:
  (
    [ | `Before(int) | `After(int)],
    [ | `Horizontal | `Vertical],
    int,
    model
  ) =>
  model;
let removeWindow: (int, model) => model;

let openEditor: (Feature_Editor.Editor.t, model) => model;

let map: (Feature_Editor.Editor.t => Feature_Editor.Editor.t, model) => model;

// UPDATE

[@deriving show]
type msg;

type outmsg =
  | Nothing
  | Focus(panel);

let update: (~focus: option(panel), model, msg) => (model, outmsg);

// VIEW

module type ContentModel = {
  type t = Feature_Editor.Editor.t;

  let id: t => int;
  let title: t => string;
  let icon: t => option(IconTheme.IconDefinition.t);
  let isModified: t => bool;

  let render: t => Revery.UI.element;
};

module View: {
  open Revery.UI;

  let make:
    (
      ~children: (module ContentModel),
      ~model: model,
      ~uiFont: UiFont.t,
      ~theme: ColorTheme.Colors.t,
      ~dispatch: msg => unit,
      unit
    ) =>
    element;

  module EditorGroupView = EditorGroupView;
};

// COMMANDS

module Commands: {
  let moveLeft: Command.t(msg);
  let moveRight: Command.t(msg);
  let moveUp: Command.t(msg);
  let moveDown: Command.t(msg);

  let rotateForward: Command.t(msg);
  let rotateBackward: Command.t(msg);

  let decreaseSize: Command.t(msg);
  let increaseSize: Command.t(msg);
  let decreaseVerticalSize: Command.t(msg);
  let increaseVerticalSize: Command.t(msg);
  let decreaseHorizontalSize: Command.t(msg);
  let increaseHorizontalSize: Command.t(msg);
  let increaseWindowSizeUp: Command.t(msg);
  let decreaseWindowSizeUp: Command.t(msg);
  let increaseWindowSizeDown: Command.t(msg);
  let decreaseWindowSizeDown: Command.t(msg);
  let increaseWindowSizeLeft: Command.t(msg);
  let decreaseWindowSizeLeft: Command.t(msg);
  let increaseWindowSizeRight: Command.t(msg);
  let decreaseWindowSizeRight: Command.t(msg);
  let maximize: Command.t(msg);
  let maximizeHorizontal: Command.t(msg);
  let maximizeVertical: Command.t(msg);
  let toggleMaximize: Command.t(msg);
  let resetSizes: Command.t(msg);
};

// CONTRIBUTIONS

module Contributions: {let commands: list(Command.t(msg));};
