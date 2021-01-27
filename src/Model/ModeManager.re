open Oni_Core;

module Internal = {
  let getTerminalNormalMode =
    fun
    | Vim.Mode.Visual(range) =>
      Mode.TerminalVisual({range: Oni_Core.VisualRange.ofVim(range)})
    | Vim.Mode.Operator({pending, _}) => Mode.Operator({pending: pending})
    | Vim.Mode.CommandLine(_) => Mode.CommandLine
    | _ => Mode.TerminalNormal;
};

let current: State.t => Oni_Core.Mode.t =
  (state: State.t) => {
    let activeEditorMode =
      state.layout |> Feature_Layout.activeEditor |> Feature_Editor.Editor.mode;
    state
    |> Selectors.getActiveTerminal
    |> Option.map((Feature_Terminal.{insertMode, _}) => {
         insertMode
           ? Mode.TerminalInsert
           : Internal.getTerminalNormalMode(activeEditorMode)
       })
    |> Option.value(
         ~default=
           switch (activeEditorMode) {
           | Vim.Mode.Insert({cursors}) =>
             Mode.Insert(
               {
                 {cursors: cursors};
               },
             )
           | Vim.Mode.Normal({cursor}) => Mode.Normal({cursor: cursor})
           | Vim.Mode.Visual(range) =>
             Mode.Visual({
               cursor: Vim.VisualRange.cursor(range),
               range: Oni_Core.VisualRange.ofVim(range),
             })
           | Vim.Mode.Select({ranges}) =>
             Mode.Select({
               ranges: ranges |> List.map(Oni_Core.VisualRange.ofVim),
             })
           | Vim.Mode.Replace({cursor}) => Mode.Replace({cursor: cursor})
           | Vim.Mode.Operator({pending, _}) =>
             Mode.Operator({pending: pending})
           | Vim.Mode.CommandLine(_) => Mode.CommandLine
           },
       );
  };
