open Oni_Core;
open Revery.UI;

type commit;

type model;

let initial: model;

[@deriving show({with_path: false})]
type msg;

let update: (model, msg) => (model, Isolinear.Effect.t(_));

module View: {
  module Full: {
    let make:
      (
        ~state: model,
        ~theme: ColorTheme.Colors.t,
        ~uiFont: UiFont.t,
        ~dispatch: msg => unit,
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
