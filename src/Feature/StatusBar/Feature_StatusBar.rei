open Oni_Core;

module Item: {
  type t;
  let create:
    (
      ~color: Exthost.Color.t=?,
      ~backgroundColor: Exthost.Color.t=?,
      ~command: string=?,
      ~tooltip: string=?,
      ~id: string,
      ~priority: int,
      ~label: Exthost.Label.t,
      ~alignment: Exthost.Msg.StatusBar.alignment=?,
      unit
    ) =>
    t;
};

// MODEL

[@deriving show]
type msg;

module Msg: {
  let itemAdded: Item.t => msg;
  let itemDisposed: string => msg;
};

type model;

let initial: model;

// UPDATE

type outmsg =
  | Nothing
  | ClearNotifications
  | ToggleProblems
  | ToggleNotifications
  | ShowFileTypePicker
  | Effect(Isolinear.Effect.t(msg));

let update: (~client: Exthost.Client.t, model, msg) => (model, outmsg);

module View: {
  let make:
    (
      ~key: Brisk_reconciler.Key.t=?,
      ~mode: Oni_Core.Mode.t,
      ~subMode: Vim.SubMode.t,
      ~notifications: Feature_Notification.model,
      ~recordingMacro: option(char),
      ~diagnostics: Feature_Diagnostics.model,
      ~font: Oni_Core.UiFont.t,
      ~activeBuffer: option(Oni_Core.Buffer.t),
      ~activeEditor: option(Feature_Editor.Editor.t),
      ~indentationSettings: Oni_Core.IndentationSettings.t,
      ~scm: Feature_SCM.model,
      ~statusBar: model,
      ~theme: ColorTheme.Colors.t,
      ~dispatch: msg => unit,
      ~workingDirectory: string,
      unit
    ) =>
    Revery.UI.element;
};

// CONFIGURATION

module Configuration: {let visible: Config.Schema.setting(bool);};

// CONTRIBUTIONS

module Contributions: {let configuration: list(Config.Schema.spec);};
