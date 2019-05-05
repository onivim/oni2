open Oni_Model;
open Revery_UI;

let component = React.component("FileExplorerView");

let createElement = (~children as _, ~state: State.t, ()) =>
  component(hooks => {
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
        | Node({data: FileSystemNode({isDirectory: false, path, _}), _}, _) =>
          GlobalContext.current().dispatch(SetExplorerTree(tree));
          /* Only open files not directories */
          GlobalContext.current().dispatch(OpenFileByPath(path));
        | Node({data: FileSystemNode({isDirectory: true, _}), _}, _) =>
          GlobalContext.current().dispatch(SetExplorerTree(tree))
        | Empty => ()
        }
      );
    };

    (
      hooks,
      <TreeView
        state
        onNodeClick
        title="Explorer"
        tree={state.fileExplorer.directory}
      />,
    );
  });
