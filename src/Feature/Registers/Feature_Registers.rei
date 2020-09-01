open Oni_Core;

type model;

let initial: model;

[@deriving show]
type msg;

module Msg: {let keyPressed: string => msg;};

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg))
  | FailedToGetRegister(char)
  | EmitRegister({
      raw: string,
      lines: array(string),
    });

let update: (msg, model) => (model, outmsg);

let sub: model => Isolinear.Sub.t(msg);

let isActive: model => bool;

module Contributions: {
  let commands: list(Command.t(msg));
  let contextKeys: list(WhenExpr.ContextKeys.Schema.entry(model));
  let keybindings: list(Oni_Input.Keybindings.keybinding);
};

module View: {
  let make:
    (
      ~key: Brisk_reconciler.Key.t=?,
      ~theme: ColorTheme.Colors.t,
      ~registers: model,
      ~font: UiFont.t,
      ~dispatch: msg => unit,
      unit
    ) =>
    Revery.UI.element;
};
