/*
 * Component_Output
 *
 * A component for showing terminal-style output
 */

open Oni_Core;

// MODEL

[@deriving show]
type msg;

[@deriving show]
type model;

let initial: model;

let set: (string, model) => model;

// UPDATE

type outmsg =
  | Nothing
  | Selected;

let update: (msg, model) => (model, outmsg);

let keyPress: (string, model) => model;

// CONTRIBUTIONS

module Contributions: {
  let commands: list(Command.t(msg));
  let keybindings: list(Feature_Input.Schema.keybinding);
  let contextKeys: model => WhenExpr.ContextKeys.t;
};

// VIEW

module View: {
  let make:
    (
      ~isActive: bool,
      ~editorFont: Service_Font.font,
      ~uiFont: UiFont.t,
      ~theme: ColorTheme.Colors.t,
      ~model: model,
      ~dispatch: msg => unit,
      unit
    ) =>
    Revery.UI.element;
};

// SUBSCRIPTION

module Sub: {let sub: model => Isolinear.Sub.t(msg);};
