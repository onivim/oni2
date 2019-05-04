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
              let name = Filename.concat(cwd, file);
              let isDirectory =
                try (Sys.is_directory(name)) {
                | Sys_error(error) =>
                  print_endline(error);
                  false;
                };

              let%lwt children =
                isDirectory
                  ? getFiles(~ignored, name, getIcon) : Lwt.return([]);

              let icon =
                isDirectory
                  ? Some(createIcon(~character=FontAwesome.folder))
                  : getIcon(name);

              TreeView.FileSystemNode({
                fullPath: name,
                displayName: file,
                isDirectory,
                children,
                icon,
              })
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

let toFilesystemNode =
  TreeView.(
    fun
    | FileSystemNode(n) => n
  );

let rec listToTree =
        (nodes: list(TreeView.treeItem), parent: TreeView.treeItem) => {
  open Tree;
  open TreeView;

  let parentId = ExplorerId.getUniqueId();
  let children =
    List.map(
      node => {
        let fsNode = toFilesystemNode(node);
        let descendants =
          List.map(
            ({children, _}) =>
              listToTree(children, FileSystemNode(fsNode)),
            List.map(toFilesystemNode, fsNode.children),
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
