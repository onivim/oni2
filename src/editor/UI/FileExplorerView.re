open Oni_Model;
open Revery_UI;
open Revery.UI.Components;

let component = React.component("FileExplorerView");

let createIcon = TreeView.toIcon(~color=Revery.Colors.white);

let getFileIcon = (languageInfo, iconTheme, filePath) => {
  let fileIcon =
    LanguageInfo.getLanguageFromFilePath(languageInfo, filePath)
    |> IconTheme.getIconForFile(iconTheme, filePath);

  switch (fileIcon) {
  | Some(icon) as x => x
  | None => None
  };
};

let rec getFiles = (cwd, getIcon) => {
  Lwt_unix.files_of_directory(cwd)
  |> Lwt_stream.map(file => {
       let name = Filename.concat(cwd, file);
       let isDirectory = Sys.is_directory(name);
       TreeView.FileSystemNode({
         fullPath: name,
         displayName: file,
         isDirectory,
         /***
            TODO: recursively call getFiles
            to get all the files and subfolder of each directory
          */
         children: isDirectory ? /* getFiles(name) */ [] : [],
         icon:
           isDirectory
             ? Some(createIcon(~character=FontAwesome.folder))
             : getIcon(name),
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

let createElement = (~children, ~state: State.t, ()) =>
  component(hooks => {
    open TreeView;
    let (tree, setDirectoryTree, hooks) =
      React.Hooks.state(Tree.Empty, hooks);

    let getIcon = getFileIcon(state.languageInfo, state.iconTheme);

    let hooks =
      React.Hooks.effect(
        OnMount,
        () => {
          let cwd = Rench.Environment.getWorkingDirectory();
          let directory = getFiles(cwd, getIcon) |> Lwt_main.run;
          let newTree =
            listToTree(
              directory,
              FileSystemNode({
                fullPath: cwd,
                displayName: Filename.basename(cwd),
                isDirectory: true,
                children: directory,
                icon: Some(createIcon(~character=FontAwesome.folderOpen)),
              }),
            );

          setDirectoryTree(newTree);
          None;
        },
        hooks,
      );
    (hooks, <TreeView tree title="File Explorer" state />);
  });
