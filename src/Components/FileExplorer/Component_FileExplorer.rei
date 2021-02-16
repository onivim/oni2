open Oni_Core;

// MODEL

[@deriving show]
type msg;

module Msg: {
  let keyPressed: string => msg;
  let activeFileChanged: option(Fp.t(Fp.absolute)) => msg;
};

type model;

let initial: (~rootPath: Fp.t(Fp.absolute)) => model;
let setRoot: (~rootPath: Fp.t(Fp.absolute), model) => model;
let root: model => Fp.t(Fp.absolute);

let keyPress: (string, model) => model;

let getFileIcon:
  (~languageInfo: Exthost.LanguageInfo.t, ~iconTheme: IconTheme.t, string) =>
  option(IconTheme.IconDefinition.t);

// UPDATE

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg))
  | OpenFile(string)
  | PreviewFile(string)
  | GrabFocus;

let update:
  (~configuration: Oni_Core.Configuration.t, msg, model) => (model, outmsg);

// SUBSCRIPTION

let sub:
  (~configuration: Oni_Core.Configuration.t, model) => Isolinear.Sub.t(msg);

// VIEW

module View: {
  let make:
    (
      ~isFocused: bool,
      ~iconTheme: IconTheme.t,
      ~languageInfo: Exthost.LanguageInfo.t,
      ~model: model,
      ~decorations: Feature_Decorations.model,
      ~theme: ColorTheme.Colors.t,
      ~font: UiFont.t,
      ~expanded: bool,
      ~onRootClicked: unit => unit,
      ~dispatch: msg => unit,
      unit
    ) =>
    Revery.UI.element;
};

module Contributions: {
  let commands: (~isFocused: bool) => list(Command.t(msg));
  let contextKeys: (~isFocused: bool, model) => WhenExpr.ContextKeys.t;
};
