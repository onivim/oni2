open Oni_Model;
open Revery_UI;
open Oni_Core.Constants;

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
      let maxDepth = default.maximumExplorerDepth;
      UiTree.(
        switch (clicked) {
        | Node({data: FileSystemNode({isDirectory: false, path, _}), _}, _) =>
          GlobalContext.current().dispatch(SetExplorerTree(tree));
          /* Only open files not directories */
          GlobalContext.current().dispatch(OpenFileByPath(path));
        /*
         * If the depth of the clicked node is at the maximum depth its children
         * will have no content so we call the update action which will populate
         * the child node and attach it to the tree

         * NOTE: that we match specifically on a Node with an empty list of children
         * so this action is not recalled if the Node has already been populated
         */
        | Node({data: FileSystemNode({isDirectory: true, depth, _}), _}, [])
            when depth == maxDepth =>
          GlobalContext.current().dispatch(UpdateExplorerNode(clicked, tree))
        | Node({data: FileSystemNode({isDirectory: true, _}), _}, _) =>
          GlobalContext.current().dispatch(SetExplorerTree(tree))
        | Empty => ()
        }
      );
    };

    (
      hooks,
      switch (state.fileExplorer.directory) {
      | Empty => React.empty
      | Node(_, _) as tree =>
        <TreeView state onNodeClick title="Explorer" tree />
      },
    );
  });
