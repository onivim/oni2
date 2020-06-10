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
  | ProviderRegistered(provider)
  | HoverInfoReceived({
      contents: list(string),
      range: option(EditorCoreTypes.Range.t),
    })
  | HoverRequestFailed(string);

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
      ~uiFont: Oni_Core.UiFont.t,
      ~editorFont: Service_Font.font,
      ~model: model,
      ~editor: Feature_Editor.Editor.t,
      ~buffer: Oni_Core.Buffer.t,
      ~gutterWidth: float,
      ~cursorOffset: int,
      ~grammars: Oni_Syntax.GrammarRepository.t,
      unit
    ) =>
    Revery.UI.element;
};
