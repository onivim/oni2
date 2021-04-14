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
  | Locations
  | Output;

[@deriving show({with_path: false})]
type msg;

type outmsg =
  | Nothing
  | PaneButton(pane)
  | OpenFile({
      filePath: string,
      position: EditorCoreTypes.CharacterPosition.t,
    })
  | PreviewFile({
      filePath: string,
      position: EditorCoreTypes.CharacterPosition.t,
    })
  | UnhandledWindowMovement(Component_VimWindows.outmsg)
  | GrabFocus
  | ReleaseFocus
  | NotificationDismissed(Feature_Notification.notification)
  | Effect(Isolinear.Effect.t(msg));

module Msg: {
  let keyPressed: string => msg;
  let resizeHandleDragged: int => msg;
  let resizeCommitted: msg;
  let toggleMessages: msg;
};

type model;

let update:
  (
    ~buffers: Feature_Buffers.model,
    ~font: Service_Font.font,
    ~languageInfo: Exthost.LanguageInfo.t,
    ~previewEnabled: bool,
    msg,
    model
  ) =>
  (model, outmsg);

module Contributions: {
  let commands: (~isFocused: bool, model) => list(Command.t(msg));
  let contextKeys: (~isFocused: bool, model) => WhenExpr.ContextKeys.t;
  let keybindings: list(Feature_Input.Schema.keybinding);
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
let setNotifications: (Feature_Notification.model, model) => model;

let setOutput: (string, option(string), model) => model;

module View: {
  let make:
    (
      ~config: Config.resolver,
      ~isFocused: bool,
      ~theme: Oni_Core.ColorTheme.Colors.t,
      ~iconTheme: Oni_Core.IconTheme.t,
      ~languageInfo: Exthost.LanguageInfo.t,
      ~editorFont: Service_Font.font,
      ~uiFont: Oni_Core.UiFont.t,
      ~dispatch: msg => unit,
      ~pane: model,
      ~workingDirectory: string,
      unit
    ) =>
    Revery.UI.element;
};
