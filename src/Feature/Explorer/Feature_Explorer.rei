open Oni_Core;

// MODEL

[@deriving show]
type msg;

module Msg: {
  let keyPressed: string => msg;
  let activeFileChanged: option(Fp.t(Fp.absolute)) => msg;
};

type model;

let initial: (~rootPath: option(Fp.t(Fp.absolute))) => model;

let setRoot: (~rootPath: option(Fp.t(Fp.absolute)), model) => model;

let root: model => option(Fp.t(Fp.absolute));

let focusOutline: model => model;

// UPDATE

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg))
  | OpenFile(string)
  | PreviewFile(string)
  | GrabFocus
  | UnhandledWindowMovement(Component_VimWindows.outmsg)
  | SymbolSelected(Feature_LanguageSupport.DocumentSymbols.symbol)
  | PickFolder;

let update:
  (~configuration: Oni_Core.Configuration.t, msg, model) => (model, outmsg);

// SUBSCRIPTION

let sub:
  (~configuration: Oni_Core.Configuration.t, model) => Isolinear.Sub.t(msg);

// VIEW

module View: {
  let make:
    (
      ~key: Brisk_reconciler.Key.t=?,
      ~isFocused: bool,
      ~iconTheme: IconTheme.t,
      ~languageInfo: Exthost.LanguageInfo.t,
      ~model: model,
      ~decorations: Feature_Decorations.model,
      ~documentSymbols: option(Feature_LanguageSupport.DocumentSymbols.t),
      ~theme: ColorTheme.Colors.t,
      ~font: UiFont.t,
      ~editorFont: Font.t,
      ~dispatch: msg => unit,
      unit
    ) =>
    Revery.UI.element;
};

module Contributions: {
  let commands: (~isFocused: bool, model) => list(Command.t(msg));
  let contextKeys: (~isFocused: bool, model) => WhenExpr.ContextKeys.t;
};
