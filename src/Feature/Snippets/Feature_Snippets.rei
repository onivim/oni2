open Oni_Core;

// Placeholder until full snippet support: Break snippet at first placeholder
let snippetToInsert: (~snippet: string) => string;


type msg;

type model;

type outmsg =
| Nothing;

let update: (msg, model) => (model, outmsg);

module Contributions: {
  let commands: list(Command.t(msg));
  //let contextKeys: (model) => WhenExpr.ContextKeys.t;
};
