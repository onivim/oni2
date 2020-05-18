open Oni_Core;
open Revery.UI;

type commit;

type model;

let initial: model;

[@deriving show({with_path: false})]
type msg;

type outmsg =
  | Nothing
  | URL(string);

let update: (model, msg) => (model, outmsg);

module View: {
  module Full: {
    let make:
      (
        ~state: model,
        ~theme: ColorTheme.Colors.t,
        ~uiFont: UiFont.t,
        ~dispatchMsg: msg => unit,
        unit
      ) =>
      element;
  };

  module Update: {
    let make:
      (~since: string, ~theme: ColorTheme.Colors.t, ~uiFont: UiFont.t, unit) =>
      element;
  };
};
