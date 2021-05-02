open Oni_Core;
open Feature_Pane.Schema;

type model = {
  tree: Component_VimTree.model(string, Oni_Components.LocationListItem.t),
};

let initial = {tree: Component_VimTree.create(~rowHeight=20)};

[@deriving show]
type msg =
  | DiagnosticsTree(Component_VimTree.msg)
  | KeyPress(string);

let update = (msg, model) =>
  switch (msg) {
  // TODO
  | DiagnosticsTree(msg) => model
  // TODO:
  | KeyPress(key) => model
  };

let contextKeys = (~isFocused, model) =>
  if (isFocused) {
    Component_VimTree.Contributions.contextKeys(model.tree);
  } else {
    WhenExpr.ContextKeys.empty;
  };

let pane: Feature_Pane.Schema.t(model, msg) =
  panel(
    ~title="Problems",
    ~id=Some("workbench.panel.markers"),
    ~commands=
      pane => {
        Component_VimTree.Contributions.commands
        |> List.map(Oni_Core.Command.map(msg => DiagnosticsTree(msg)))
      },
    ~contextKeys,
    ~view=
      (~config, ~font, ~isFocused, ~theme, ~dispatch, ~model) =>
        Revery.UI.React.empty,
    ~keyPressed=key => KeyPress(key),
  );

