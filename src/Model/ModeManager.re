open Oni_Core;

module Internal = {
  let getTerminalNormalMode =
    fun
    | Vim.Mode.Visual({range, _}) =>
      Mode.TerminalVisual({range: Oni_Core.VisualRange.ofVim(range)})
    | Vim.Mode.Operator({pending, _}) => Mode.Operator({pending: pending})
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
           | Vim.Mode.Insert({cursors}) =>
             Mode.Insert(
               {
                 {cursors: cursors};
               },
             )
           | Vim.Mode.Normal({cursor}) => Mode.Normal({cursor: cursor})
           | Vim.Mode.Visual({range}) =>
             Mode.Visual({
               cursor: Vim.VisualRange.cursor(range),
               range: Oni_Core.VisualRange.ofVim(range),
             })
           | Vim.Mode.Select({range}) =>
             Mode.Select({
               cursor: Vim.VisualRange.cursor(range),
               range: Oni_Core.VisualRange.ofVim(range),
             })
           | Vim.Mode.Replace({cursor}) => Mode.Replace({cursor: cursor})
           | Vim.Mode.Operator({pending, _}) =>
             Mode.Operator({pending: pending})
           | Vim.Mode.CommandLine => Mode.CommandLine
           },
       );
