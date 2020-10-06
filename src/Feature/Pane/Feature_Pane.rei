/*
 * Feature_Pane.rei
 *
 * Feature for the bottom pane, hosting notifications, diagnostics, etc.
 */
open Oni_Core;

[@deriving show({with_path: false})]
type pane =
  | Diagnostics
  | Notifications
  | Locations;

[@deriving show({with_path: false})]
type msg;

type outmsg =
  | Nothing
  | OpenFile({
      filePath: string,
      position: EditorCoreTypes.CharacterPosition.t,
    })
  | UnhandledWindowMovement(Component_VimWindows.outmsg)
  | GrabFocus
  | ReleaseFocus
  | Effect(Isolinear.Effect.t(msg));

module Msg: {
  let keyPressed: string => msg;
  let resizeHandleDragged: int => msg;
  let resizeCommitted: msg;
};

type model;

let update:
  (
    ~buffers: Feature_Buffers.model,
    ~font: Service_Font.font,
    ~languageInfo: Exthost.LanguageInfo.t,
    msg,
    model
  ) =>
  (model, outmsg);

module Contributions: {
  let commands: (~isFocused: bool, model) => list(Command.t(msg));
  let contextKeys: (~isFocused: bool, model) => WhenExpr.ContextKeys.t;
  let keybindings: list(Oni_Input.Keybindings.keybinding);
};

let initial: model;

let height: model => int;
let selected: model => pane;
let isOpen: model => bool;

let setPane: (~pane: pane, model) => model;
let show: (~pane: pane, model) => model;
let toggle: (~pane: pane, model) => model;
let close: model => model;

let setDiagnostics: (Feature_Diagnostics.model, model) => model;
let setLocations:
  (
    ~maybeActiveBuffer: option(Oni_Core.Buffer.t),
    ~locations: list(Exthost.Location.t),
    model
  ) =>
  model;

module View: {
  let make:
    (
      ~key: Brisk_reconciler.Key.t=?,
      ~config: Config.resolver,
      ~isFocused: bool,
      ~theme: Oni_Core.ColorTheme.Colors.t,
      ~iconTheme: Oni_Core.IconTheme.t,
      ~languageInfo: Exthost.LanguageInfo.t,
      ~uiFont: Oni_Core.UiFont.t,
      ~notifications: Feature_Notification.model,
      ~dispatch: msg => unit,
      ~notificationDispatch: Feature_Notification.msg => unit,
      ~pane: model,
      ~workingDirectory: string,
      unit
    ) =>
    Revery.UI.element;
};
