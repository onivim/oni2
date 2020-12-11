open Oni_Core;

// Placeholder until full snippet support: Break snippet at first placeholder
let snippetToInsert: (~snippet: string) => string;

type msg;

type model;

type outmsg =
  | Nothing;

module Snippet: {
  type t;

  let parse: string => result(t, string);
};

// module Session: {
//   type t;

//   let start: (
//     ~editorId: int,
//     ~buffer: Buffer.t,
//     ~position: BytePosition.t,
//     ~snippet: Snippet.t
//   ) => Isolinear.Effect.t(msg);
// }

let update: (msg, model) => (model, outmsg);

module Contributions: {
  let commands: list(Command.t(msg));
  let contextKeys: model => WhenExpr.ContextKeys.t;
};
