/*
 * Feature_Pane.rei
 *
 * Feature for the bottom pane, hosting notifications, diagnostics, etc.
 */
open Oni_Core;

[@deriving show({with_path: false})]
type pane =
  | Diagnostics
  | Notifications;

[@deriving show({with_path: false})]
type msg;

type outmsg =
  | Nothing
  | OpenFile({
      filePath: string,
      position: EditorCoreTypes.CharacterPosition.t,
    })
  | UnhandledWindowMovement(Component_VimWindows.outmsg);

module Msg: {
  let keyPressed: string => msg;
  let resizeHandleDragged: int => msg;
  let resizeCommitted: msg;
};

type model;

let update: (msg, model) => (model, outmsg);

module Contributions: {
  let commands: (~isFocused: bool) => list(Command.t(msg));
  let contextKeys: (~isFocused: bool) => WhenExpr.ContextKeys.Schema.t(model);
  let keybindings: list(Oni_Input.Keybindings.keybinding);
};

let initial: model;

let height: model => int;
let selected: model => pane;
let isVisible: (pane, model) => bool;
let isOpen: model => bool;

let show: (~pane: pane, model) => model;
let toggle: (~pane: pane, model) => model;
let close: model => model;

module View: {
  let make:
    (
      ~isFocused: bool,
      ~theme: Oni_Core.ColorTheme.Colors.t,
      ~uiFont: Oni_Core.UiFont.t,
      ~editorFont: Service_Font.font,
      ~diagnostics: Feature_LanguageSupport.Diagnostics.t,
      ~notifications: Feature_Notification.model,
      ~dispatch: msg => unit,
      ~notificationDispatch: Feature_Notification.msg => unit,
      ~pane: model,
      unit
    ) =>
    Revery.UI.element;
};
