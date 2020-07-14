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
  | InsertRegister
  | Cancel
  | Commit;

[@deriving show]
type msg =
  | RegisterNotAvailable(char)
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
  | FailedToGetRegister(char)
  | EmitRegister({
      raw: string,
      lines: array(string),
    });

let handleTextInput = (model, key) =>
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
          | None => RegisterNotAvailable(char)
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
    let inputText' =
      Feature_InputText.handleInput(~key, expression.inputText);
    (
      {mode: ExpressionRegister({...expression, inputText: inputText'})},
      Nothing,
    );
  };

let update = (msg, model) => {
  switch (msg) {
  | Command(InsertRegister) => ({mode: WaitingForRegister}, Nothing)
  | Command(Cancel) => (initial, Nothing)
  | Command(Commit) =>
    switch (model.mode) {
    | ExpressionRegister({evaluation, _}) =>
      switch (evaluation) {
      | Ok(result) => (
          initial,
          EmitRegister({raw: result, lines: [|result|]}),
        )
      | Error(msg) => (initial, Nothing)
      }

    | WaitingForRegister => (initial, Nothing)
    | NotActive => (initial, Nothing)
    }

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
  | RegisterNotAvailable(c) => (model, FailedToGetRegister(c))
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
      "vim.register.insert",
      Command(InsertRegister),
    );

  let cancel = define("vim.register.cancel", Command(Cancel));

  let commit = define("vim.register.commit", Command(Commit));
};

module ContextKeys = {
  open WhenExpr.ContextKeys.Schema;

  let registerEvaluationFocus = bool("registerEvaluationFocus", isActive);
};

module Keybindings = {
  open Oni_Input.Keybindings;

  let condition = "registerEvaluationFocus" |> WhenExpr.parse;

  let cancel = {key: "<ESC>", command: Commands.cancel.id, condition};

  let commit = {key: "<CR>", command: Commands.commit.id, condition};

  let insert = {
    key: "<C-R>",
    command: Commands.insert.id,
    condition:
      "insertMode || commandLineMode || inQuickOpen || terminalFocus"
      |> WhenExpr.parse,
  };
};

module Contributions = {
  let commands = [Commands.insert, Commands.cancel, Commands.commit];

  let contextKeys = [ContextKeys.registerEvaluationFocus];

  let keybindings = [
    Keybindings.cancel,
    Keybindings.commit,
    Keybindings.insert,
  ];
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
