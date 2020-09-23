open Revery.UI;

let make =
    (~isFocused: bool,~model, ~decorations, ~theme, ~font, ~dispatch: Model.msg => unit, ()) => {
  switch ((model: Model.model)) {
  | {tree: Some(tree), treeView, active, focus, scrollOffset, _} =>
    let onNodeClick = node => dispatch(NodeClicked(node));

    <FileTreeView
      isFocused
      scrollOffset
      decorations
      active
      focus
      onNodeClick
      tree
      treeView
      theme
      font
      dispatch
    />;
  | _ => React.empty
  };
};
