open Oni_Core;

// MODEL

[@deriving show]
type kind =
  | Success
  | Info
  | Warning
  | Error;

[@deriving show]
type notification = {
  id: int,
  kind,
  message: string,
  source: option(string),
};

type model = list(notification);

let initial: model;

// UPDATE

[@deriving show]
type msg;

let update: (model, msg) => model;

// EFFECTS

module Effects: {
  let create:
    (~kind: kind=?, ~source: string=?, string) => Isolinear.Effect.t(msg);
  let dismiss: notification => Isolinear.Effect.t(msg);
  let dismissAll: Isolinear.Effect.t(msg);
};

// COLORS

module Colors: {
  let backgroundFor: notification => ColorTheme.Schema.definition;
  let foregroundFor: notification => ColorTheme.Schema.definition;
};

// VIEW

module View: {
  open Revery;
  open Revery.UI;

  module Popup: {
    module Animations: {
      let transitionDuration: Time.t;
      let totalDuration: Time.t;
    };

    let make:
      (
        ~key: React.Key.t=?,
        ~model: notification,
        ~background: Color.t,
        ~foreground: Color.t,
        ~font: UiFont.t,
        unit
      ) =>
      React.element(React.node);
  };

  module List: {
    let make:
      (
        ~model: model,
        ~theme: ColorTheme.Colors.t,
        ~font: UiFont.t,
        ~dispatch: msg => unit,
        unit
      ) =>
      React.element(React.node);
  };
};

// CONTRIBUTIONS

module Contributions: {let colors: list(ColorTheme.Schema.definition);};
