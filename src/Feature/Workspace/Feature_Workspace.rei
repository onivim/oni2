open Oni_Core;

// MODEL

type model;

[@deriving show]
type msg;

module Msg: {let workingDirectoryChanged: string => msg;};

let initial: (~openedFolder: option(string), string) => model;

let openedFolder: model => option(string);

let workingDirectory: model => string;

let rootName: model => string;

// UPDATE

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg))
  | WorkspaceChanged({
      path: option(string),
      shouldFocusExplorer: bool,
    });

let update: (msg, model) => (model, outmsg);

// EFFECTS

module Effects: {
  let changeDirectory: FpExp.t(FpExp.absolute) => Isolinear.Effect.t(msg);
  let pickFolder: Isolinear.Effect.t(msg);
};

// CONTRIBUTIONS

module Contributions: {
  let commands: model => list(Command.t(msg));

  let menuGroup: MenuBar.Schema.group;
};
