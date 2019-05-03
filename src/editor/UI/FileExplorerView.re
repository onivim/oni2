open Revery_UI;
open Revery.UI.Components;

let component = React.component("FileExplorerView");

let rec getFiles = cwd => {
  Lwt_unix.files_of_directory(cwd)
  |> Lwt_stream.map(file => {
       let name = Filename.concat(cwd, file);
       let isDirectory = Sys.is_directory(name);
       TreeView.{
         name,
         hasChildren: isDirectory,
         children: isDirectory ? /* getFiles(name) */ [] : [],
       };
     })
  |> Lwt_stream.to_list;
};

module ExplorerId =
  Revery.UniqueId.Make({});

let rec listToTree = (nodes, parent) => {
  Tree.(
    List.fold_left(
      (tree, node) =>
        switch (tree) {
        | Node(x, children) =>
          Node(
            x,
            [
              Node(
                {id: ExplorerId.getUniqueId(), data: node, status: Closed},
                TreeView.[listToTree(node.children, node)],
              ),
              ...children,
            ],
          )
        | Empty => tree
        },
      Node({id: 0, data: parent, status: Open}, []),
      nodes,
    )
  );
};

let createElement = (~children, ~state, ()) =>
  component(hooks => {
    open TreeView;
    let (tree, setDirectoryTree, hooks) =
      React.Hooks.state(Tree.Empty, hooks);
    let hooks =
      React.Hooks.effect(
        OnMount,
        () => {
          let cwd = Rench.Environment.getWorkingDirectory();
          let directory = getFiles(cwd) |> Lwt_main.run;
          let newTree =
            listToTree(
              directory,
              {name: cwd, hasChildren: true, children: directory},
            );

          setDirectoryTree(newTree);
          None;
        },
        hooks,
      );
    (hooks, <TreeView tree title="File Explorer" state />);
  });
