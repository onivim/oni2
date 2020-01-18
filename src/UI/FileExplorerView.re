open Oni_Model;
open Revery_UI;

let make = (~state: State.t, ()) => {
  let onNodeClick = node =>
    GlobalContext.current().dispatch(
      FileExplorer(FileExplorer.NodeClicked(node)),
    );

  switch (state.fileExplorer) {
  | {tree: None, _} => React.empty
  | {tree: Some(tree), active, focus, _} =>
    <FileTreeView state active focus onNodeClick tree />
  };
};
