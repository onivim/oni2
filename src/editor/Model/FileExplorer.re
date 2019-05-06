open Revery;
open Oni_Core;

type t = {
  directory: UiTree.t,
  isOpen: bool,
};

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

let createFsNode = (~children, ~path, ~displayName, ~fileIcon, ~isDirectory) => {
  /**
     TODO: Find an icon theme with folders and use those icons
     Fallbacks are used for the directory icons. FontAwesome is not
     accessible here so we specify no icons for directories.
   */
  let (primary, secondary) = isDirectory ? (None, None) : (fileIcon, None);

  UiTree.FileSystemNode({
    path,
    displayName,
    children,
    isDirectory,
    icon: primary,
    secondaryIcon: secondary,
  });
};

/**
  getFilesAndFolders

   This function uses Lwt to get all the files and folders in a directory
   then for each we check if it is a file, if so we create a filesystem
   node (an in-memory representation of the file) without children
   if it is a directory we recursively call getFilesAndFolders on it
   to resolves its subfolders and files. We do this concurrently using
   Lwt_list.map_p

   TODO: Add the concept of depth to this function so we can terminate
   the file resolution early and avoid choking on large directories
 */
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
    Printf.sprintf(
      "Error: %s encountered in %s called with %s",
      Unix.error_message(error),
      fn,
      arg,
    )
    |> Log.error;
    Lwt.return([]);
  };
};

let rec listToTree = (~status, nodes, parent) => {
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
              listToTree(
                ~status=Closed,
                descendant.children,
                FileSystemNode(descendant),
              ),
            descendantNodes,
          );

        let id = ExplorerId.getUniqueId();
        Node({id, data: node, status: Closed}, descendants);
      },
      nodes,
    );

  Node({id: parentId, data: parent, status}, children);
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
  |> listToTree(~status=Open, directory);
};

let create = () => {directory: UiTree.Empty, isOpen: true};

let reduce = (state: t, action: Actions.t) => {
  switch (action) {
  | SetExplorerTree(tree) => {directory: tree, isOpen: true}
  | RemoveDockItem(WindowManager.ExplorerDock) => {...state, isOpen: false}
  | _ => state
  };
};
