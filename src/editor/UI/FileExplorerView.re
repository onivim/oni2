open Oni_Model;
open Oni_Core;
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

let checkIsDirectory = dir =>
  try (Sys.is_directory(dir)) {
  | Sys_error(error) =>
    print_endline(error);
    false;
  };

let createFsNode = (~children, ~path, ~displayName, ~fileIcon, ~isDirectory) => {
  let (primary, secondary) =
    isDirectory
      ? (
        Some(createIcon(~character=FontAwesome.folder)),
        Some(createIcon(~character=FontAwesome.folderOpen)),
      )
      : (fileIcon, None);

  TreeView.FileSystemNode({
    path,
    displayName,
    children,
    isDirectory,
    icon: primary,
    secondaryIcon: secondary,
  });
};

let rec getFiles = (cwd, getIcon, ~ignored) => {
  try%lwt (
    Lwt_unix.files_of_directory(cwd)
    /* Filter out the relative name for current and parent directory*/
    |> Lwt_stream.filter(name => name != ".." && name != ".")
    /* Remove ignored files from search */
    |> Lwt_stream.filter(name => !List.mem(name, ignored))
    |> Lwt_stream.to_list
    |> (
      promise =>
        Lwt.bind(promise, files =>
          Lwt_list.map_p(
            file => {
              let path = Filename.concat(cwd, file);
              let isDirectory = checkIsDirectory(path);

              let%lwt children =
                isDirectory
                  ? getFiles(~ignored, path, getIcon) : Lwt.return([]);

              createFsNode(
                ~path,
                ~displayName=file,
                ~children,
                ~isDirectory,
                ~fileIcon=getIcon(path),
              )
              |> Lwt.return;
            },
            files,
          )
        )
    )
  )
    {
    | Failure(e) =>
      print_endline(e);
      Lwt.return([]);
    };
    /* | Unix.Unix_error(_, _, message) => */
    /*   print_endline("Error: " ++ message); */
    /*   Lwt.return([]); */
};

module ExplorerId =
  Revery.UniqueId.Make({});

let toFsNode =
  TreeView.(
    fun
    | FileSystemNode(n) => n
  );

let rec listToTree = (nodes, parent) => {
  open Tree;
  let parentId = ExplorerId.getUniqueId();
  let children =
    List.map(
      node => {
        open TreeView;
        let n = toFsNode(node);
        let descendantNodes = List.map(toFsNode, n.children);

        let descendants =
          List.map(
            c => listToTree(c.children, FileSystemNode(c)),
            descendantNodes,
          );

        let id = ExplorerId.getUniqueId();
        Node({id, data: node, status: Closed}, descendants);
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
          let ignored = ["node_modules", "_esy"];
          let directory = getFiles(~ignored, cwd, getIcon) |> Lwt_main.run;
          let newTree =
            createFsNode(
              ~path=cwd,
              ~displayName=Filename.basename(cwd),
              ~isDirectory=true,
              ~children=directory,
              ~fileIcon=getIcon(cwd),
            )
            |> listToTree(directory);

          setDirectoryTree(newTree);
          None;
        },
        hooks,
      );

    let onNodeClick = (clicked, tree) => {
      setDirectoryTree(tree);
      Tree.(
        switch (clicked) {
        | Node({data}, _) =>
          let node = toFsNode(data);
          GlobalContext.current().dispatch(OpenFileByPath(node.path));
        | Empty => ()
        }
      );
    };

    (hooks, <TreeView tree state title="File Explorer" onNodeClick />);
  });
