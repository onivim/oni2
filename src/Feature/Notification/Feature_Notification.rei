open Oni_Core;

// MODEL

[@deriving show]
type kind =
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

type model; //= list(notification);

let initial: model;

let active: model => list(notification);

let all: model => list(notification);

// UPDATE

[@deriving show]
type msg;

let update: (model, msg) => model;

// EFFECTS

module Effects: {
  let create:
    (~kind: kind=?, ~source: string=?, string) => Isolinear.Effect.t(msg);
  let dismiss: notification => Isolinear.Effect.t(msg);
};

// SUBSCRIPTION

let sub: model => Isolinear.Sub.t(msg);

// COLORS

module Colors: {
  let backgroundFor: notification => ColorTheme.Schema.definition;
  let foregroundFor: notification => ColorTheme.Schema.definition;
};

// ANIMATIONS

module Animations: {
  let transitionDuration: Revery.Time.t;
  let totalDuration: Revery.Time.t;
};

// VIEW

module View: {
  open Revery;
  open Revery.UI;

  module Popup: {
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
