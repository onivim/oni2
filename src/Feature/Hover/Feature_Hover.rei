type model;

let initial: model;

[@deriving show({with_path: false})]
type command;

[@deriving show({with_path: false})]
type msg =
  | Command(command)
  | KeyPressed(string);

let update: (model, msg) => model;

module Contributions: {let commands: list(Oni_Core.Command.t(msg));};

module View: {
  let make:
    (
      ~colorTheme: Oni_Core.ColorTheme.Colors.t,
      ~tokenTheme: Oni_Syntax.TokenTheme.t,
      ~languageInfo: Oni_Extensions.LanguageInfo.t,
      ~fontFamily: Revery.Font.Family.t,
      ~codeFontFamily: Revery.Font.Family.t,
      ~model: model,
      unit
    ) =>
    Revery.UI.element;
};
