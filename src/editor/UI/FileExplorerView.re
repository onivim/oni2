open Revery_UI;
open Revery.UI.Components;

let component = React.component("FileExplorerView");

let rec getFiles = cwd => {
  Lwt_unix.files_of_directory(cwd)
  |> Lwt_stream.map(file => {
       let name = Filename.concat(cwd, file);
       let isDirectory = Sys.is_directory(name);
       TreeView.FileSystemNode({
         fullPath: name,
         displayName: file,
         isDirectory,
         children: isDirectory ? /* getFiles(name) */ [] : [],
         icon: Some(isDirectory ? FontAwesome.folder : FontAwesome.file),
       });
     })
  |> Lwt_stream.to_list;
};

module ExplorerId =
  Revery.UniqueId.Make({});

let listToTree = (nodes, parent) => {
  open Tree;
  open TreeView;

  let parentId = ExplorerId.getUniqueId();
  let children =
    List.map(
      node => {
        let id = ExplorerId.getUniqueId();
        /**
           TODO: This should recursively create a list of
           children which are themselves trees
         */
        Node({id, data: node, status: Closed}, []);
      },
      nodes,
    );

  Node({id: parentId, data: parent, status: Open}, children);
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
              FileSystemNode({
                fullPath: cwd,
                displayName: Filename.basename(cwd),
                isDirectory: true,
                children: directory,
                icon: Some(FontAwesome.folderOpen),
              }),
            );

          setDirectoryTree(newTree);
          None;
        },
        hooks,
      );
    (hooks, <TreeView tree title="File Explorer" state />);
  });
