open Oni_Core;
open Feature_Pane.Schema;

type model = {
  tree: Component_VimTree.model(string, Oni_Components.LocationListItem.t),
};

let initial = {tree: Component_VimTree.create(~rowHeight=20)};

[@deriving show]
type msg =
  | DiagnosticsTree(Component_VimTree.msg);

let update = (_msg, model) => model;

let contextKeys = (~isFocused, model) => {
  // TODO
  WhenExpr.ContextKeys.empty;
};

let pane: Feature_Pane.Schema.t(model, msg) =
  panel(
    ~title="Problems",
    ~id=Some("workbench.panel.markers"),
    ~commands=_ => [],
    ~contextKeys,
    ~view=
      (~config, ~font, ~isFocused, ~theme, ~dispatch, ~model) =>
        Revery.UI.React.empty,
    ~keyPressed=key => failwith("TODO"),
  );

