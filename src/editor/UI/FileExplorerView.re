open Oni_Model;
open Oni_Core;
open Revery_UI;
open Revery.UI.Components;

let component = React.component("FileExplorerView");

let createElement = (~children, ~state: State.t, ()) =>
  component(hooks => {
    open TreeView;
    let hooks =
      React.Hooks.effect(
        OnMount,
        () => {
          let cwd = Rench.Environment.getWorkingDirectory();
          GlobalContext.current().dispatch(OpenExplorer(cwd));
          None;
        },
        hooks,
      );

    let onNodeClick = (clicked, tree) => {
      Tree.(
        switch (clicked) {
        | Node({data}, _) =>
          let node = FileExplorer.toFsNode(data);
          GlobalContext.current().dispatch(SetExplorerTree(tree));
          GlobalContext.current().dispatch(OpenFileByPath(node.path));
        | Empty => ()
        }
      );
    };

    (
      hooks,
      <TreeView
        state
        tree={state.fileExplorer.directory}
        title="Explorer"
        onNodeClick
      />,
    );
  });
