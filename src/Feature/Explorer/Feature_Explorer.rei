open Oni_Core;

// MODEL

[@deriving show]
type msg;

module Msg: {
  let keyPressed: string => msg;
  let activeFileChanged: option(string) => msg;
};

type model;

let initial: model;

// UPDATE

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg))
  | OpenFile(string)
  | GrabFocus;

let update:
  (
    ~configuration: Oni_Core.Configuration.t,
    ~languageInfo: Exthost.LanguageInfo.t,
    ~iconTheme: Oni_Core.IconTheme.t,
    msg,
    model
  ) =>
  (model, outmsg);

module View: {
  let make:
    (
      ~model: model,
      ~decorations: Feature_Decorations.model,
      ~theme: ColorTheme.Colors.t,
      ~font: UiFont.t,
      ~dispatch: msg => unit,
      unit
    ) =>
    Revery.UI.element;
};
