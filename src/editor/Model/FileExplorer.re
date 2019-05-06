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

let createFsNode =
    (~children, ~depth, ~path, ~displayName, ~fileIcon, ~isDirectory) => {
  /**
     TODO: Find an icon theme with folders and use those icons
     Fallbacks are used for the directory icons. FontAwesome is not
     accessible here so we specify no icons for directories.
   */
  let (primary, secondary) = isDirectory ? (None, None) : (fileIcon, None);

  UiTree.FileSystemNode({
    path,
    depth,
    displayName,
    children,
    isDirectory,
    icon: primary,
    secondaryIcon: secondary,
  });
};

let isDir = path => Sys.file_exists(path) ? Sys.is_directory(path) : false;

let printUnixError = (error, fn, arg) =>
  Printf.sprintf(
    "Error: %s encountered in %s called with %s",
    Unix.error_message(error),
    fn,
    arg,
  )
  |> Log.error;

let handleError = (~defaultValue, func) => {
  try%lwt (func()) {
  | Unix.Unix_error(error, fn, arg) =>
    printUnixError(error, fn, arg);
    Lwt.return(defaultValue);
  | Failure(e) =>
    Log.error(e);
    Lwt.return(defaultValue);
  };
};

/**
  getFilesAndFolders

   This function uses Lwt to get all the files and folders in a directory
   then for each we check if it is a file or folder.
   if it is a directory we recursively call getFilesAndFolders on it
   to resolves its subfolders and files. We do this concurrently using
   Lwt_list.map_p. The recursion is gated by the depth value so it does
   not recurse too far.
 */
let getFilesAndFolders = (~maxDepth, ~ignored, cwd, getIcon) => {
  let rec getDirContent = (~depth, cwd, getIcon) => {
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
              let isDirectory = isDir(path);
              let nextDepth = depth + 1;

              /**
                 If resolving children for a particular directory fails
                 log the error but carry on processing other directories
                 TODO: add hasError prop to a node so we can indicate there
                 was a problem resolving a directory
               */
              let%lwt children =
                isDirectory && nextDepth < maxDepth
                  ? handleError(
                      () => getDirContent(~depth=nextDepth, path, getIcon),
                      ~defaultValue=[],
                    )
                  : Lwt.return([]);

              createFsNode(
                ~path,
                ~children,
                ~isDirectory,
                ~depth=nextDepth,
                ~displayName=file,
                ~fileIcon=getIcon(path),
              )
              |> Lwt.return;
            },
            files,
          )
        )
    );
  };

  handleError(() => getDirContent(~depth=0, cwd, getIcon), ~defaultValue=[]);
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
  let maxDepth = Constants.default.maximumExplorerDepth;
  let directory =
    getFilesAndFolders(~maxDepth, ~ignored, cwd, getIcon) |> Lwt_main.run;

  createFsNode(
    ~depth=0,
    ~path=cwd,
    ~displayName=Filename.basename(cwd),
    ~isDirectory=true,
    ~children=directory,
    ~fileIcon=getIcon(cwd),
  )
  |> listToTree(~status=Open, directory);
};

let getNodePath = node => {
  UiTree.(
    switch (node) {
    | Node({data: FileSystemNode({path, _}), _}, _) => Some(path)
    | Empty => None
    }
  );
};

let getNodeId = node => {
  UiTree.(
    switch (node) {
    | Node({id, _}, _) => Some(id)
    | Empty => None
    }
  );
};

let create = () => {directory: UiTree.Empty, isOpen: true};

let reduce = (state: t, action: Actions.t) => {
  switch (action) {
  | SetExplorerTree(tree) => {directory: tree, isOpen: true}
  | RemoveDockItem(WindowManager.ExplorerDock) => {...state, isOpen: false}
  | _ => state
  };
};
