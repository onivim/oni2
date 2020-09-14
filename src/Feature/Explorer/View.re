open Revery.UI;

let make = (~model, ~theme, ~font, ~dispatch: Model.msg => unit, ()) => {
  switch ((model: Model.model)) {
  | {tree: Some(tree), active, focus, scrollOffset, decorations, _} =>
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
