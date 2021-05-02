/*
 * Feature_Pane.rei
 *
 * Feature for the bottom pane, hosting notifications, diagnostics, etc.
 */
open Oni_Core;

module Schema: {
  type t('model, 'msg);

  let panel:
    (
      ~title: string,
      ~id: option(string),
      ~contextKeys: (~isFocused: bool, 'model) => WhenExpr.ContextKeys.t,
      ~commands: 'model => list(Command.t('msg)),
      ~view: (
               ~config: Config.resolver,
               ~font: UiFont.t,
               ~isFocused: bool,
               ~iconTheme: IconTheme.t,
               ~languageInfo: Exthost.LanguageInfo.t,
               ~workingDirectory: string,
               ~theme: ColorTheme.Colors.t,
               ~dispatch: 'msg => unit,
               ~model: 'model
             ) =>
             Revery.UI.element,
      ~keyPressed: string => 'msg
    ) =>
    t('model, 'msg);

  let map:
    (~msg: 'msgA => 'msgB, ~model: 'modelA => 'modelB, t('modelB, 'msgA)) =>
    t('modelA, 'msgB);
};

// [@deriving show({with_path: false})]
// type pane =
//   | Diagnostics
//   | Notifications
//   | Locations
//   | Output;

[@deriving show({with_path: false})]
type msg('inner);

module Msg: {
  let keyPressed: string => msg(_);
  let resizeHandleDragged: int => msg(_);
  let resizeCommitted: msg(_);
  let toggleMessages: msg(_);
};

type outmsg('msg) =
  | Nothing
  | NestedMessage('msg)
  | UnhandledWindowMovement(Component_VimWindows.outmsg)
  | GrabFocus
  | ReleaseFocus;

type model('model, 'msg);

let update:
  (
    ~buffers: Feature_Buffers.model,
    ~font: Service_Font.font,
    ~languageInfo: Exthost.LanguageInfo.t,
    ~previewEnabled: bool,
    msg('msg),
    model('model, 'msg)
  ) =>
  (model('model, 'msg), outmsg('msg));

module Contributions: {
  let commands:
    (~isFocused: bool, 'model, model('model, 'msg)) =>
    list(Command.t(msg('msg)));
  let contextKeys:
    (~isFocused: bool, 'model, model('model, 'msg)) => WhenExpr.ContextKeys.t;
  let keybindings: list(Feature_Input.Schema.keybinding);
};

let initial: list(Schema.t('model, 'msg)) => model('model, 'msg);

let height: model(_, _) => int;
//let selected: model(_, _) => pane;
let isOpen: model(_, _) => bool;

//let setPane: (~pane: pane, model) => model;
//let show: (~pane: pane, model) => model;
//let toggle: (~pane: pane, model) => model;

let close: model('model, 'msg) => model('model, 'msg);

// let setDiagnostics: (Feature_Diagnostics.model, model) => model;
// let setLocations:
//   (
//     ~maybeActiveBuffer: option(Oni_Core.Buffer.t),
//     ~locations: list(Exthost.Location.t),
//     model
//   ) =>
//   model;
// let setNotifications: (Feature_Notification.model, model) => model;

// let setOutput: (string, option(string), model) => model;

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
      ~dispatch: msg('msg) => unit,
      ~pane: model('model, 'msg),
      ~model: 'model,
      ~workingDirectory: string,
      unit
    ) =>
    Revery.UI.element;
};
