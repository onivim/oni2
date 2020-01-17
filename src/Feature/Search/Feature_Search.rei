open EditorCoreTypes;
open Oni_Core;
open Revery.UI;

type model;

let initial: model;

[@deriving show]
type msg =
  | Input(string)
  | InputClicked(int)
  | Update([@opaque] list(Ripgrep.Match.t))
  | Complete;

type outmsg =
  | Focus;

let update: (model, msg) => (model, option(outmsg));

let subscriptions:
  (Ripgrep.t, msg => unit, model) => list(Subscription.t(msg));

let make:
  (
    ~theme: Theme.t,
    ~uiFont: UiFont.t,
    ~editorFont: EditorFont.t,
    ~isFocused: bool,
    ~model: model,
    ~onSelectResult: (string, Location.t) => unit,
    ~dispatch: msg => unit,
    unit
  ) =>
  React.element(React.node);
