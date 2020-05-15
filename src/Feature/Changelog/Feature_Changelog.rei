open Oni_Core;
open Revery.UI;

[@deriving show({with_path: false})]
type msg =
  | PullRequestClicked(int)
  | CommitHashClicked(string);

module View: {
  module Full: {
    let make:
      (
        ~theme: ColorTheme.Colors.t,
        ~uiFont: UiFont.t,
        ~onPullRequestClicked: int => unit,
        ~onCommitHashClicked: string => unit,
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
