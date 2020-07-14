open Oni_Core;

let initialText = Feature_InputText.create(~placeholder="Type expression...");

type mode =
  | NotActive
  | WaitingForRegister
  | ExpressionRegister({
      inputText: Feature_InputText.model,
      evaluation: result(string, string),
    });

type model = {mode};

let initial = {mode: NotActive};

let isActive = ({mode}) => mode != NotActive;

[@deriving show]
type command =
  | InsertRegister;

[@deriving show]
type msg =
  | RegisterNotAvailable
  | RegisterAvailable({
      raw: string,
      lines: array(string),
    })
  | Command(command)
  | KeyPressed(string)
  | ExpressionEvaluated(string)
  | ExpressionError(string)
  | InputText(Feature_InputText.msg);

module Msg = {
  let keyPressed = key => KeyPressed(key);
};

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg))
  | EmitRegister({
      raw: string,
      lines: array(string),
    });

let handleTextInput = (model, key) =>
  if (key == "<ESC>") {
    (initial, Nothing);
  } else {
    switch (model.mode) {
    | NotActive =>
      // TODO: This shouldn't happen
      (model, Nothing)
    | WaitingForRegister =>
      if (String.length(key) == 1) {
        let char = key.[0];

        if (char == '=') {
          (
            {
              mode:
                ExpressionRegister({
                  inputText: initialText,
                  evaluation: Ok(""),
                }),
            },
            Nothing,
          );
        } else {
          let toMsg = (
            fun
            | None => RegisterNotAvailable
            | Some(lines) => {
                let raw = lines |> Array.to_list |> String.concat("\n");
                RegisterAvailable({raw, lines});
              }
          );
          let eff = Service_Vim.Effects.getRegisterValue(~toMsg, char);
          ({mode: NotActive}, Effect(eff));
        };
      } else {
        (initial, Nothing);
      }
    | ExpressionRegister(expression) =>
      if (key == "<ESC>") {
        (initial, Nothing);
      } else if (key == "<CR>") {
        let eff =
          expression.evaluation
          |> Result.map(eval => {
               EmitRegister({raw: eval, lines: [|eval|]})
             })
          |> Result.value(~default=Nothing);
        (initial, eff);
      } else {
        let inputText' =
          Feature_InputText.handleInput(~key, expression.inputText);
        (
          {mode: ExpressionRegister({...expression, inputText: inputText'})},
          Nothing,
        );
      }
    };
  };

let update = (msg, model) => {
  switch (msg) {
  | Command(InsertRegister) => ({mode: WaitingForRegister}, Nothing)

  | KeyPressed(key) => handleTextInput(model, key)
  | InputText(msg) =>
    let model' =
      switch (model.mode) {
      | ExpressionRegister(expression) =>
        let inputText' = Feature_InputText.update(msg, expression.inputText);
        {mode: ExpressionRegister({...expression, inputText: inputText'})};
      | NotActive => model
      | WaitingForRegister => model
      };
    (model', Nothing);

  | ExpressionEvaluated(result) =>
    let mode =
      switch (model.mode) {
      | ExpressionRegister({inputText, _}) =>
        ExpressionRegister({inputText, evaluation: Ok(result)})
      | NotActive => NotActive
      | WaitingForRegister => WaitingForRegister
      };
    ({mode: mode}, Nothing);
  | RegisterAvailable({raw, lines}) => (model, EmitRegister({raw, lines}))
  | RegisterNotAvailable => (model, Nothing)
  | ExpressionError(err) =>
    let mode =
      switch (model.mode) {
      | ExpressionRegister({inputText, _}) =>
        ExpressionRegister({inputText, evaluation: Error(err)})
      | NotActive => NotActive
      | WaitingForRegister => WaitingForRegister
      };
    ({mode: mode}, Nothing);
  };
};

module Commands = {
  open Feature_Commands.Schema;

  let insert =
    define(
      ~category="Vim",
      ~title="Insert register",
      "vim.insertRegister",
      Command(InsertRegister),
    );
};

module Contributions = {
  let commands = [Commands.insert];
};

module Styles = {
  open Revery;
  open Revery.UI;
  open Style;
  let color = Color.rgba(0., 0., 0., 0.75);
  let boxShadow = [
    backgroundColor(color),
    boxShadow(
      ~xOffset=4.,
      ~yOffset=4.,
      ~blurRadius=12.,
      ~spreadRadius=0.,
      ~color,
    ),
  ];
};

module View = {
  open Revery.UI;
  module Colors = Feature_Theme.Colors;
  let%component make =
                (
                  ~theme: ColorTheme.Colors.t,
                  ~registers: model,
                  ~font: UiFont.t,
                  ~dispatch: msg => unit,
                  (),
                ) => {
    let%hook () = Hooks.effect(OnMount, () => None);

    switch (registers.mode) {
    | NotActive
    // Could potentially have a helper UI here - something to show the available registers!
    | WaitingForRegister => React.empty
    | ExpressionRegister({inputText, evaluation}) =>
      <View style=Styles.boxShadow>
        <View
          style=Style.[
            width(400),
            backgroundColor(Colors.Menu.background.from(theme)),
            color(Colors.Menu.foreground.from(theme)),
          ]>
          <View style=Style.[padding(5)]>
            <Feature_InputText.View
              prefix="="
              model=inputText
              theme
              isFocused=true
              fontFamily={font.family}
              fontSize=14.
              dispatch={msg => dispatch(InputText(msg))}
            />
          </View>
          <View style=Style.[padding(5), flexDirection(`RowReverse)]>
            {let (text, textColor) =
               switch (evaluation) {
               | Ok(v) => (v, Colors.Menu.foreground.from(theme))
               | Error(msg) => (
                   msg,
                   Colors.EditorError.foreground.from(theme),
                 )
               };
             <Text
               style=Style.[color(textColor)]
               text
               fontFamily={font.family}
               fontSize=12.
             />}
          </View>
        </View>
      </View>
    };
  };
};

let sub = registers => {
  switch (registers.mode) {
  | WaitingForRegister
  | NotActive => Isolinear.Sub.none
  | ExpressionRegister({inputText, _}) =>
    let toMsg = (
      fun
      | Ok(str) => ExpressionEvaluated(str)
      | Error(err) => ExpressionError(err)
    );
    Service_Vim.Sub.eval(~toMsg, inputText |> Feature_InputText.value);
  };
};
