// MODEL

type model;

let initial: model;

let mode: model => Vim.Mode.t;

// MSG

[@deriving show]
type msg =
  | ModeChanged([@opaque] Vim.Mode.t);

// UPDATE

let update: (msg, model) => model;
