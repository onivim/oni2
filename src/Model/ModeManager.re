open Oni_Core;
let current: State.t => Oni_Core.Mode.t =
  (state: State.t) =>
    state
    |> Selectors.getActiveTerminal
    |> Option.map(({normalMode, _}: BufferRenderer.terminal) => {
         normalMode ? Mode.TerminalNormal : Mode.TerminalInsert
       })
    |> Option.value(
         ~default=
           switch (state.vimMode) {
           | Vim.Types.Insert => Mode.Insert
           | Vim.Types.Normal => Mode.Normal
           | Vim.Types.Visual => Mode.Visual
           | Vim.Types.Select => Mode.Select
           | Vim.Types.Replace => Mode.Replace
           | Vim.Types.Operator => Mode.Operator
           | Vim.Types.CommandLine => Mode.CommandLine
           },
       );
