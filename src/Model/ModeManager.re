open Oni_Core;

module Internal = {
  let getTerminalNormalMode =
    fun
    | Vim.Mode.Visual({range}) =>
      Mode.TerminalVisual({range: Oni_Core.VisualRange.ofVim(range)})
    | Vim.Mode.Operator({pending}) => Mode.Operator({pending: pending})
    | Vim.Mode.CommandLine => Mode.CommandLine
    | _ => Mode.TerminalNormal;
};

let current: State.t => Oni_Core.Mode.t =
  (state: State.t) =>
    state
    |> Selectors.getActiveTerminal
    |> Option.map((Feature_Terminal.{insertMode, _}) => {
         insertMode
           ? Mode.TerminalInsert
           : Internal.getTerminalNormalMode(Feature_Vim.mode(state.vim))
       })
    |> Option.value(
         ~default=
           switch (Feature_Vim.mode(state.vim)) {
           | Vim.Mode.Insert => Mode.Insert
           | Vim.Mode.Normal => Mode.Normal
           | Vim.Mode.Visual({range}) =>
             Mode.Visual({range: Oni_Core.VisualRange.ofVim(range)})
           | Vim.Mode.Select({range}) =>
             Mode.Select({range: Oni_Core.VisualRange.ofVim(range)})
           | Vim.Mode.Replace => Mode.Replace
           | Vim.Mode.Operator({pending}) =>
             Mode.Operator({pending: pending})
           | Vim.Mode.CommandLine => Mode.CommandLine
           },
       );
