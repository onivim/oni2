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
  | ToggleVisibility;

[@deriving show]
type msg =
  | ResizeInProgress(int)
  | ResizeCommitted
  | Command(command)
  | FileExplorerClicked
  | SearchClicked
  | SCMClicked
  | ExtensionsClicked;

type outmsg =
  | Nothing
  | Focus
  | PopFocus;

let update: (~isFocused: bool, msg, model) => (model, outmsg);

let initial: model;

let width: model => int;
let isOpen: model => bool;
let selected: model => pane;
let location: model => location;

let isVisible: (pane, model) => bool;
let toggle: (pane, model) => model;

let configurationChanged: (~config: Oni_Core.Config.resolver, model) => model;

module Contributions: {
  let commands: list(Command.t(msg));
  let configuration: list(Config.Schema.spec);
  let keybindings: list(Oni_Input.Keybindings.keybinding);
  let contextKeys:
    (~isFocused: bool) => list(WhenExpr.ContextKeys.Schema.entry(model));
};
