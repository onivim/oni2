open Oni_Model;
open Revery_UI;

module Option = Oni_Core.Utility.Option;

let%component make = (~state: State.t, ()) => {
  let%hook () =
    React.Hooks.effect(
      OnMount,
      () => {
        let cwd = Rench.Environment.getWorkingDirectory();
        GlobalContext.current().dispatch(OpenExplorer(cwd));
        None;
      },
    );

  let onNodeClick = node =>
    GlobalContext.current().dispatch(ExplorerNodeClicked(node));

  switch (state.fileExplorer.directory) {
  | None => React.empty
  | Some(tree) => <FileTreeView state onNodeClick title="Explorer" tree />
  };
};
