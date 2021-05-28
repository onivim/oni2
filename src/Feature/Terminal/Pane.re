// type t = option(terminal);
// [@deriving show]
// type msg =
//   | KeyPress(string);
// type outmsg =
//   | Nothing;
// let initial = {terminal: None};
// let commands = _model => [];
// let contextKeys = (~isFocused, model) => {
// TODO
//   WhenExpr.ContextKeys.empty;
// isFocused
//   ? model.output
//     |> Option.map(Component_Output.Contributions.contextKeys)
//     |> Option.value(~default=WhenExpr.ContextKeys.empty)
//   : WhenExpr.ContextKeys.empty;
// };
// let update = (msg, model) => {
//   (model, Nothing);
// } /* }*/;
// let pane: Feature_Pane.Schema.t(t, msg) = {
//   Feature_Pane.Schema.(
//     panel(
//       ~title="Terminal",
//       ~id=Some("workbench.panel.terminal"),
//       ~commands,
//       ~contextKeys,
//       ~view=
//         (
//           ~getTerminalById,
//           ~config as _,
//           ~editorFont,
//           ~font,
//           ~isFocused,
//           ~iconTheme as _,
//           ~languageInfo as _,
//           ~workingDirectory as _,
//           ~theme,
//           ~dispatch,
//           ~model,
//         ) => {
//           model.terminalId
//           |> OptionEx.flatMap(getTerminalById)
//           getTerminalById(model.)
//           <TerminalView
//           Revery.UI.React.empty,
//         },
//       ~keyPressed=key => KeyPress(key),
//     )
/*   )*/
