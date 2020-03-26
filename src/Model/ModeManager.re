type t =
  // Vim modes
  | Normal
  | Insert
  | Visual
  | Select
  | Replace
  | Operator
  | CommandLine
  // Additional modes
  | TerminalInsert;

let current = (state: State.t) =>
  if (Selectors.terminalIsActive(state)) {
    TerminalInsert;
                  // TODO: Wire up terminal-normal mode
  } else {
    switch (state.mode) {
    | Vim.Types.Insert => Insert
    | Vim.Types.Normal => Normal
    | Vim.Types.Visual => Visual
    | Vim.Types.Select => Select
    | Vim.Types.Replace => Replace
    | Vim.Types.Operator => Operator
    | Vim.Types.CommandLine => CommandLine
    };
  };
