open Revery;
open Oni_Core;

type t = {directory: UiTree.t};

module ExplorerId =
  UniqueId.Make({});

let toFsNode = node =>
  switch (node) {
  | UiTree.FileSystemNode(n) => n
  };

let getFileIcon = (languageInfo, iconTheme, filePath) => {
  let fileIcon =
    LanguageInfo.getLanguageFromFilePath(languageInfo, filePath)
    |> IconTheme.getIconForFile(iconTheme, filePath);

  switch (fileIcon) {
  | Some(_) as x => x
  | None => None
  };
};

let toIcon = (~character, ~color) =>
  IconTheme.IconDefinition.{fontCharacter: character, fontColor: color};

let createFsNode = (~children, ~path, ~displayName, ~fileIcon, ~isDirectory) => {
  let icon = toIcon(~color=Colors.white);
  let (primary, secondary) =
    isDirectory
      ? (
        Some(icon(~character=FontAwesome.folder)),
        Some(icon(~character=FontAwesome.folderOpen)),
      )
      : (fileIcon, None);

  UiTree.FileSystemNode({
    path,
    displayName,
    children,
    isDirectory,
    icon: primary,
    secondaryIcon: secondary,
  });
};

let rec getFilesAndFolders = (cwd, getIcon, ~ignored) => {
  /* Wrap the operation in a try%lwt which will catch all async exceptions */
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
              let isDirectory = Utility.checkIsDirectory(path);

              let%lwt children =
                isDirectory
                  ? getFilesAndFolders(~ignored, path, getIcon)
                  : Lwt.return([]);

              createFsNode(
                ~path,
                ~children,
                ~isDirectory,
                ~displayName=file,
                ~fileIcon=getIcon(path),
              )
              |> Lwt.return;
            },
            files,
          )
        )
    )
  ) {
  | Failure(e) =>
    Log.error(e);
    Lwt.return([]);
  | Unix.Unix_error(error, fn, arg) =>
    let message =
      "Error: "
      ++ Unix.error_message(error)
      ++ " encountered in "
      ++ fn
      ++ " called with "
      ++ arg;
    Log.error(message);
    Lwt.return([]);
  };
};

let rec listToTree = (nodes, parent) => {
  open UiTree;
  let parentId = ExplorerId.getUniqueId();
  let children =
    List.map(
      node => {
        let fsNode = toFsNode(node);
        let descendantNodes = List.map(toFsNode, fsNode.children);

        let descendants =
          List.map(
            descendant =>
              listToTree(descendant.children, FileSystemNode(descendant)),
            descendantNodes,
          );

        let id = ExplorerId.getUniqueId();
        Node({id, data: node, status: Closed}, descendants);
      },
      nodes,
    );

  Node({id: parentId, data: parent, status: Open}, children);
};

let getDirectoryTree = (cwd, languageInfo, iconTheme, ignored) => {
  let getIcon = getFileIcon(languageInfo, iconTheme);
  let directory = getFilesAndFolders(~ignored, cwd, getIcon) |> Lwt_main.run;

  createFsNode(
    ~path=cwd,
    ~displayName=Filename.basename(cwd),
    ~isDirectory=true,
    ~children=directory,
    ~fileIcon=getIcon(cwd),
  )
  |> listToTree(directory);
};

let create = () => {directory: UiTree.Empty};

let reduce = (state: t, action: Actions.t) => {
  switch (action) {
  | SetExplorerTree(tree) => {directory: tree}
  | _ => state
  };
};
