open Oni_Core;
open Feature_Editor;

// MODEL

type panel =
  | Left
  | Center
  | Bottom;

type model;

let initial: list(Editor.t) => model;

let visibleEditors: model => list(Editor.t);
let editorById: (int, model) => option(Editor.t);

let split: ([ | `Horizontal | `Vertical], model) => model;

let activeEditor: model => Editor.t;

let openEditor: (Editor.t, model) => model;
let closeBuffer: (~force: bool, Vim.Types.buffer, model) => option(model);

let map: (Editor.t => Editor.t, model) => model;

// UPDATE

[@deriving show]
type msg;

type outmsg =
  | Nothing
  | SplitAdded
  | RemoveLastBlocked
  | Focus(panel);

let update: (~focus: option(panel), model, msg) => (model, outmsg);

// VIEW

module type ContentModel = {
  type t = Editor.t;

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
      ~isZenMode: bool,
      ~shouldShowTabsInZenMode: bool,
      ~uiFont: UiFont.t,
      ~theme: ColorTheme.Colors.t,
      ~dispatch: msg => unit,
      unit
    ) =>
    element;
};

// COMMANDS

module Commands: {
  let nextEditor: Command.t(msg);
  let previousEditor: Command.t(msg);

  let splitVertical: Command.t(msg);
  let splitHorizontal: Command.t(msg);

  let closeActiveEditor: Command.t(msg);

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
