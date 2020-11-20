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

let update: (msg, model) => model;

// EFFECTS

module Effects: {
  let changeDirectory: Fp.t(Fp.absolute) => Isolinear.Effect.t(msg);
};
