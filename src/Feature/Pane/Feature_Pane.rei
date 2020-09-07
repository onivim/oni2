/*
 * Feature_Pane.rei
 *
 * Feature for the bottom pane, hosting notifications, diagnostics, etc.
 */

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
    });

module Msg: {
  let resizeHandleDragged: int => msg;
  let resizeCommitted: msg;
};

type model;

let update: (msg, model) => (model, outmsg);

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
