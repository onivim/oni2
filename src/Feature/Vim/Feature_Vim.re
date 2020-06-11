// MODEL

type model = {mode: Vim.Mode.t};

let initial = {mode: Vim.Types.Normal};

let mode = ({mode}) => mode;

// MSG

[@deriving show]
type msg =
  | ModeChanged([@opaque] Vim.Mode.t);

let update = (msg, _model: model) => {
  switch (msg) {
  | ModeChanged(mode) => ({mode: mode}: model)
  };
};
