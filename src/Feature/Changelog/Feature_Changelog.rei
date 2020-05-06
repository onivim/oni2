open Oni_Core;
open Revery.UI;

module View: {
  module Full: {
    let make:
      (~theme: ColorTheme.Colors.t, ~uiFont: UiFont.t, unit) => element;
  };

  module Update: {
    let make:
      (~since: string, ~theme: ColorTheme.Colors.t, ~uiFont: UiFont.t, unit) =>
      element;
  };
};
