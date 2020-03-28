let current: State.t => Oni_Core.Mode.t =
  (state: State.t) =>
    if (Selectors.terminalIsActive(state)) {
      TerminalInsert;
    } else {
      switch (state.vimMode) {
      | Vim.Types.Insert => Insert
      | Vim.Types.Normal => Normal
      | Vim.Types.Visual => Visual
      | Vim.Types.Select => Select
      | Vim.Types.Replace => Replace
      | Vim.Types.Operator => Operator
      | Vim.Types.CommandLine => CommandLine
      };
    };
