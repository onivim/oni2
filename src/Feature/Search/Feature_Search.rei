open EditorCoreTypes;
open Oni_Core;
open Revery.UI;

type model;

let initial: model;

[@deriving show]
type msg =
  | Input(string)
  | Pasted(string)
  | Update([@opaque] list(Ripgrep.Match.t))
  | Complete
  | SearchError(string)
  | FindInput(Feature_InputText.msg);

type outmsg =
  | Focus;

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
    ~onSelectResult: (string, Location.t) => unit,
    ~dispatch: msg => unit,
    unit
  ) =>
  React.element(React.node);
