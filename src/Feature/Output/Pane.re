open Oni_Core;

type t = {output: option(Component_Output.model)};

[@deriving show]
type msg =
  | Output(Component_Output.msg)
  | KeyPress(string);

type outmsg =
  | Nothing
  | ClosePane;

let initial = {output: None};

let commands = _model => [];

let contextKeys = (~isFocused, model) => {
  isFocused
    ? model.output
      |> Option.map(Component_Output.Contributions.contextKeys)
      |> Option.value(~default=WhenExpr.ContextKeys.empty)
    : WhenExpr.ContextKeys.empty;
};

let set = (~cmd as _, ~maybeContents, model) => {
  let output' =
    maybeContents
    |> Option.map(output =>
         Component_Output.set(output, Component_Output.initial)
       );
  {output: output'};
};

let update = (msg, model) => {
  switch (msg) {
  | KeyPress(key) => (
      {output: model.output |> Option.map(Component_Output.keyPress(key))},
      Nothing,
    )

  | Output(outputMsg) =>
    model.output
    |> Option.map(outputPane => {
         let (outputPane, outmsg) =
           Component_Output.update(outputMsg, outputPane);
         let model' = {...model, output: Some(outputPane)};
         switch (outmsg) {
         | Nothing => (model', Nothing)
         // Emulate Vim behavior on space enter close;
         | Selected => (model, ClosePane)
         };
       })
    |> Option.value(~default=(model, Nothing))
  };
};

module View = {
  open Revery.UI;
  let make = (~model, ~isActive, ~editorFont, ~uiFont, ~theme, ~dispatch, ()) => {
    model.output
    |> Option.map(model => {
         <Component_Output.View
           model
           isActive
           editorFont
           uiFont
           theme
           dispatch={msg => dispatch(Output(msg))}
         />
       })
    |> Option.value(~default=React.empty);
  };
};

let pane = {
  Feature_Pane.Schema.(
    panel(
      ~title="Output",
      ~id=Some("workbench.panel.output"),
      ~commands,
      ~contextKeys,
      ~view=
        (
          ~config,
          ~editorFont,
          ~font,
          ~isFocused,
          ~iconTheme,
          ~languageInfo,
          ~workingDirectory,
          ~theme,
          ~dispatch,
          ~model,
        ) =>
          <View
            editorFont
            uiFont=font
            isActive=isFocused
            theme
            dispatch
            model
          />,
      ~keyPressed=key => KeyPress(key),
    )
  );
};
