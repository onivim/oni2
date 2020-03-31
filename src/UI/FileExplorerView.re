open Oni_Model;
open Revery.UI;

let make = (~model, ~theme, ~font, ()) => {
  switch ((model: FileExplorer.t)) {
  | {tree: Some(tree), active, focus, scrollOffset, decorations, _} =>
    let onNodeClick = node =>
      GlobalContext.current().dispatch(
        FileExplorer(FileExplorer.NodeClicked(node)),
      );

    <FileTreeView
      scrollOffset
      decorations
      active
      focus
      onNodeClick
      tree
      theme
      font
    />;
  | _ => React.empty
  };
};
