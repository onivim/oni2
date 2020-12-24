open Oni_Core;

[@deriving show({with_path: false})]
type provider = {
  handle: int,
  selector: Exthost.DocumentSelector.t,
  metadata: Exthost.SignatureHelp.ProviderMetadata.t,
};

type model;

let initial: model;

[@deriving show({with_path: false})]
type msg;

module Msg: {let providerAvailable: provider => msg;};

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg))
  | Error(string);

let isShown: model => bool;

module Commands: {
  let incrementSignature: Command.t(msg);
  let decrementSignature: Command.t(msg);
};

let update:
  (
    ~maybeBuffer: option(Buffer.t),
    ~maybeEditor: option(Feature_Editor.Editor.t),
    model,
    msg
  ) =>
  (model, outmsg);

let startInsert:
  (~config: Config.resolver, ~maybeBuffer: option(Buffer.t), model) => model;
let stopInsert: model => model;

let bufferUpdated:
  (
    ~languageConfiguration: LanguageConfiguration.t,
    ~buffer: Buffer.t,
    ~activeCursor: EditorCoreTypes.CharacterPosition.t,
    model
  ) =>
  model;

let sub:
  (
    ~buffer: Buffer.t,
    ~isInsertMode: bool,
    ~activePosition: EditorCoreTypes.CharacterPosition.t,
    ~client: Exthost.Client.t,
    model
  ) =>
  Isolinear.Sub.t(msg);

module Contributions: {
  let commands: list(Command.t(msg));
  let keybindings: list(Feature_Input.Schema.keybinding);
  let configuration: list(Config.Schema.spec);
};

module View: {
  let make:
    (
      ~colorTheme: ColorTheme.Colors.t,
      ~tokenTheme: Oni_Syntax.TokenTheme.t,
      ~languageInfo: Exthost.LanguageInfo.t,
      ~uiFont: UiFont.t,
      ~editorFont: Service_Font.font,
      ~model: model,
      ~buffer: Oni_Core.Buffer.t,
      ~editor: Feature_Editor.Editor.t,
      ~gutterWidth: float,
      ~grammars: Oni_Syntax.GrammarRepository.t,
      ~dispatch: msg => unit,
      unit
    ) =>
    Revery.UI.element;
};
