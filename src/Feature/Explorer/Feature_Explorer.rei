open Oni_Core;

// MODEL

[@deriving show]
type msg;

module Msg: {
  let keyPressed: string => msg;
  let activeFileChanged: option(string) => msg;
};

type model;

let initial: (~rootPath: string) => model;

let setRoot: (~rootPath: string, model) => model;

let focusOutline: model => model;

// UPDATE

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg))
  | OpenFile(string)
  | GrabFocus
  | UnhandledWindowMovement(Component_VimWindows.outmsg)
  | SymbolSelected(Feature_LanguageSupport.DocumentSymbols.symbol);

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
      ~dispatch: msg => unit,
      unit
    ) =>
    Revery.UI.element;
};

module Contributions: {
  let commands: (~isFocused: bool, model) => list(Command.t(msg));
  let contextKeys: (~isFocused: bool, model) => WhenExpr.ContextKeys.t;
};
