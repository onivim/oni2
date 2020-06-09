[@deriving show({with_path: false})]
type provider = {
  handle: int,
  selector: list(Exthost.DocumentFilter.t),
};

type model;

let initial: model;

[@deriving show({with_path: false})]
type command;

[@deriving show({with_path: false})]
type msg =
  | Command(command)
  | KeyPressed(string)
  | ProviderRegistered(provider);

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg));

let update:
  (
    ~maybeBuffer: option(Oni_Core.Buffer.t),
    ~maybeEditor: option(Feature_Editor.Editor.t),
    ~extHostClient: Exthost.Client.t,
    model,
    msg
  ) =>
  (model, outmsg);

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
