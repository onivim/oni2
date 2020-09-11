open EditorCoreTypes;
open Oni_Core;
open Revery.UI;

type model;

let initial: model;

[@deriving show]
type msg;

module Msg: {
  let input: string => msg;
  let pasted: string => msg;
};

type outmsg =
  | Focus
  | UnhandledWindowMovement(Component_VimWindows.outmsg);

let update: (model, msg) => (model, option(outmsg));

let subscriptions:
  (Ripgrep.t, msg => unit, model) => list(Subscription.t(msg));

let make:
  (
    ~theme: ColorTheme.Colors.t,
    ~uiFont: UiFont.t,
    ~editorFont: Service_Font.font,
    ~isFocused: bool,
    ~model: model,
    ~onSelectResult: (string, CharacterPosition.t) => unit,
    ~dispatch: msg => unit,
    unit
  ) =>
  React.element(React.node);

module Contributions: {
  let commands: (~isFocused: bool) => list(Command.t(msg));
  let contextKeys: (~isFocused: bool) => WhenExpr.ContextKeys.Schema.t(model);
};
