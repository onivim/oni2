open Revery.UI;

let make =
    (~model, ~decorations, ~theme, ~font, ~dispatch: Model.msg => unit, ()) => {
  switch ((model: Model.model)) {
  | {tree: Some(tree), active, focus, scrollOffset, _} =>
    let onNodeClick = node => dispatch(NodeClicked(node));

    <FileTreeView
      scrollOffset
      decorations
      active
      focus
      onNodeClick
      tree
      theme
      font
      dispatch
    />;
  | _ => React.empty
  };
};
