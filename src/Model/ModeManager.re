open Oni_Core;

module Internal = {
  let getTerminalNormalMode =
    fun
    | Vim.Types.Visual => Mode.TerminalVisual
    | Vim.Types.Operator => Mode.Operator
    | Vim.Types.CommandLine => Mode.CommandLine
    | _ => Mode.TerminalNormal;
};

let current: State.t => Oni_Core.Mode.t =
  (state: State.t) =>
    state
    |> Selectors.getActiveTerminal
    |> Option.map(({insertMode, _}: BufferRenderer.terminal) => {
         insertMode
           ? Mode.TerminalInsert
           : Internal.getTerminalNormalMode(state.vimMode)
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
