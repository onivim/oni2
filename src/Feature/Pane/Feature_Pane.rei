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
      ~sub: (~isFocused: bool, 'model) => Isolinear.Sub.t('msg),
      ~title: string,
      ~id: option(string),
      ~buttons: (
                  ~font: UiFont.t,
                  ~theme: ColorTheme.Colors.t,
                  ~dispatch: 'msg => unit,
                  ~model: 'model
                ) =>
                Revery.UI.element,
      ~contextKeys: (~isFocused: bool, 'model) => WhenExpr.ContextKeys.t,
      ~commands: 'model => list(Command.t('msg)),
      ~view: (
               ~config: Config.resolver,
               ~editorFont: Service_Font.font,
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

[@deriving show({with_path: false})]
type msg('inner);

module Msg: {
  let keyPressed: string => msg(_);
  let resizeHandleDragged: int => msg(_);
  let resizeCommitted: msg(_);

  let toggle: (~paneId: string) => msg(_);
  let close: (~paneId: string) => msg(_);
};

type outmsg('msg) =
  | Nothing
  | NestedMessage('msg)
  | UnhandledWindowMovement(Component_VimWindows.outmsg)
  | GrabFocus
  | ReleaseFocus;

type model('model, 'msg);

let update:
  (msg('msg), model('model, 'msg)) => (model('model, 'msg), outmsg('msg));

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
let isOpen: model(_, _) => bool;

let show: (~paneId: string, model('model, 'msg)) => model('model, 'msg);

let close: model('model, 'msg) => model('model, 'msg);

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

let sub:
  (~isFocused: bool, 'model, model('model, 'msg)) =>
  Isolinear.Sub.t(msg('msg));
