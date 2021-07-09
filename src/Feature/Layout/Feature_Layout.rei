open Oni_Core;
open Feature_Editor;

// MODEL

[@deriving show]
type panel =
  | Left
  | Right
  | Center
  | Bottom;

module Group: {
  type t;

  type id;

  let id: t => id;

  let allEditors: t => list(Editor.t);
};

module LayoutTab: {
  type t;

  let groups: t => list(Group.t);
};

type model;

let activeGroup: model => Group.t;
let activeLayoutGroups: model => list(Group.t);
let setActiveGroup: (Group.id, model) => model;

let layouts: model => list(LayoutTab.t);
let activeLayout: model => LayoutTab.t;

let initial: list(Editor.t) => model;

let visibleEditors: model => list(Editor.t);
let editorById: (int, model) => option(Editor.t);
let removeEditor: (int, model) => option(model);

let split:
  (
    ~shouldReuse: bool,
    ~editor: Editor.t,
    [ | `Horizontal | `Vertical],
    model
  ) =>
  model;

let activeEditor: model => Editor.t;
let activeGroupEditors: model => list(Editor.t);

let openEditor: (~config: Config.resolver, Editor.t, model) => model;
let closeBuffer: (~force: bool, Vim.Types.buffer, model) => option(model);

let addLayoutTab: (~editor: Editor.t, model) => model;
let gotoLayoutTab: (int, model) => model;
let previousLayoutTab: (~count: int=?, model) => model;
let nextLayoutTab: (~count: int=?, model) => model;
let removeLayoutTab: (int, model) => option(model);
let removeLayoutTabRelative: (~delta: int, model) => option(model);
let removeActiveLayoutTab: model => option(model);
let removeOtherLayoutTabs: model => model;
let removeOtherLayoutTabsRelative: (~count: int, model) => model;
let moveActiveLayoutTabTo: (int, model) => model;
let moveActiveLayoutTabRelative: (int, model) => model;

let map: (Editor.t => Editor.t, model) => model;
let fold: (('acc, Editor.t) => 'acc, 'acc, model) => 'acc;

// UPDATE

[@deriving show]
type msg;

module Msg: {
  let moveLeft: msg;
  let moveRight: msg;
  let moveUp: msg;
  let moveDown: msg;
};

type outmsg =
  | Nothing
  | SplitAdded
  | RemoveLastWasBlocked
  | Focus(panel);

let update: (~focus: option(panel), model, msg) => (model, outmsg);

// VIEW

module View: {
  open Revery.UI;

  module type ContentModel = {
    type t = Editor.t;

    let id: t => int;
    let title: t => string;
    let preview: t => bool;
    let tooltip: t => string;
    let icon: t => option(IconTheme.IconDefinition.t);
    let isModified: t => bool;

    let render: (~isActive: bool, t) => Revery.UI.element;
  };

  let make:
    (
      ~children: (module ContentModel),
      ~model: model,
      ~isFocused: bool,
      ~isZenMode: bool,
      ~showTabs: bool,
      ~config: Config.resolver,
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
  let closeActiveSplit: Command.t(msg);
  let closeActiveSplitUnlessLast: Command.t(msg);

  let moveLeft: Command.t(msg);
  let moveRight: Command.t(msg);
  let moveUp: Command.t(msg);
  let moveDown: Command.t(msg);
  let moveTopLeft: Command.t(msg);
  let moveBottomRight: Command.t(msg);
  let cycleForward: Command.t(msg);
  let cycleBackward: Command.t(msg);

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

  let addLayout: Command.t(msg);
  let previousLayout: Command.t(msg);
  let nextLayout: Command.t(msg);
};

// CONTRIBUTIONS

module Contributions: {
  let commands: list(Command.t(msg));
  let configuration: list(Config.Schema.spec);
};
