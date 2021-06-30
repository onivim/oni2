open Oni_Core;

type pane =
  | FileExplorer
  | SCM
  | Extensions
  | Search;

type location =
  | Left
  | Right;

// MODEL

type model;

// UPDATE

[@deriving show]
type command =
  | ToggleExplorerPane
  | ToggleSearchPane
  | ToggleSCMPane
  | ToggleExtensionsPane
  | ToggleVisibility
  | GotoOutline;

[@deriving show]
type msg =
  | ResizeInProgress(int)
  | ResizeCommitted
  | Command(command)
  | FileExplorerClicked
  | SearchClicked
  | SCMClicked
  | ExtensionsClicked;

type subFocus =
  | Outline;

type outmsg =
  | Nothing
  | Focus(option(subFocus))
  | PopFocus;

let update: (~isFocused: bool, msg, model) => (model, outmsg);

let initial: model;

let width: model => int;
let isOpen: model => bool;
let isOpenByDefault: model => bool;
let selected: model => pane;
let location: model => location;

let isVisible: (pane, model) => bool;
let toggle: (pane, model) => model;
let show: model => model;

let configurationChanged:
  (~hasWorkspace: bool, ~config: Oni_Core.Config.resolver, model) => model;

module Contributions: {
  let commands: list(Command.t(msg));
  let configuration: list(Config.Schema.spec);
  let keybindings: list(Feature_Input.Schema.keybinding);
  let menuGroups: list(ContextMenu.Schema.group);
  let contextKeys:
    (~isFocused: bool) => list(WhenExpr.ContextKeys.Schema.entry(model));
};
