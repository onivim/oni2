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
      UiTree.(
        switch (clicked) {
        | Node({data: FileSystemNode({isDirectory: false, path, _})}, _) =>
          GlobalContext.current().dispatch(SetExplorerTree(tree));
          /* Only open files not directories */
          GlobalContext.current().dispatch(OpenFileByPath(path));
        | Node({data: FileSystemNode({isDirectory: true, _})}, _) =>
          GlobalContext.current().dispatch(SetExplorerTree(tree))
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
