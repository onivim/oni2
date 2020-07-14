open Oni_Core;

[@deriving show({with_path: false})]
type provider = {
  handle: int,
  selector: list(Exthost.DocumentFilter.t),
  metadata: Exthost.SignatureHelp.ProviderMetadata.t,
};

type model;

let initial: model;

[@deriving show({with_path: false})]
type command =
  | Show
  | IncrementSignature
  | DecrementSignature;

[@deriving show({with_path: false})]
type msg =
  | Command(command)
  | ProviderRegistered(provider)
  | KeyPressed(option(string), bool)
  | InfoReceived({
      signatures: list(Exthost.SignatureHelp.Signature.t),
      activeSignature: int,
      activeParameter: int,
      requestID: int,
      editorID: int,
    })
  | EmptyInfoReceived(int)
  | RequestFailed(string)
  | SignatureIncrementClicked
  | SignatureDecrementClicked;

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
    ~extHostClient: Exthost.Client.t,
    model,
    msg
  ) =>
  (model, outmsg);

module Contributions: {let commands: list(Command.t(msg));};

module View: {
  let make:
    (
      ~colorTheme: ColorTheme.Colors.t,
      ~tokenTheme: Oni_Syntax.TokenTheme.t,
      ~languageInfo: Exthost.LanguageInfo.t,
      ~uiFont: UiFont.t,
      ~editorFont: Service_Font.font,
      ~model: model,
      ~editor: Feature_Editor.Editor.t,
      ~gutterWidth: float,
      ~grammars: Oni_Syntax.GrammarRepository.t,
      ~dispatch: msg => unit,
      unit
    ) =>
    Revery.UI.element;
};
