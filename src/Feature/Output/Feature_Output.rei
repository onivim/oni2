[@deriving show]
type msg;

type model;

let initial: model;

type outmsg =
  | Nothing
  | ClosePane;

let update: (msg, model) => (model, outmsg);

let setProcessOutput: (~cmd: string, ~output: option(string), model) => model;

module Contributions: {let pane: Feature_Pane.Schema.t(model, msg);};
