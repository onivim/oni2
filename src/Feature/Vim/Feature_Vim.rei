// MODEL

type model;

let initial: model;

let mode: model => Vim.Mode.t;

// MSG

[@deriving show]
type msg =
  | ModeChanged([@opaque] Vim.Mode.t)
  | PasteCompleted({cursors: [@opaque] list(Vim.Cursor.t)})
  | Pasted(string);

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg))
  | CursorsUpdated(list(Vim.Cursor.t));

// UPDATE

let update: (msg, model) => (model, outmsg);

module CommandLine: {let getCompletionMeet: string => option(int);};
