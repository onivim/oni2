open Revery;
open Oni_Core;

type t = {
  directory: option(UiTree.t),
  isOpen: bool,
};

module ExplorerId =
  UniqueId.Make({});

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
    (~depth, ~path, ~displayName, ~fileIcon as icon, ~isDirectory) => {
  UiTree.{path, depth, displayName, isDirectory, icon};
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
  try%lwt(func()) {
  | Unix.Unix_error(error, fn, arg) =>
    printUnixError(error, fn, arg);
    Lwt.return(defaultValue);
  | Failure(e) =>
    Log.error(e);
    Lwt.return(defaultValue);
  };
};

let sortByLoweredDisplayName = (a: UiTree.t, b: UiTree.t) => {
  switch (a.data.isDirectory, b.data.isDirectory) {
  | (true, false) => (-1)
  | (false, true) => 1
  | _ =>
    compare(
      a.data.displayName |> String.lowercase_ascii,
      b.data.displayName |> String.lowercase_ascii,
    )
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
  let attempt = handleError(~defaultValue=[]);
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
              let parentId = ExplorerId.getUniqueId();

              /**
                 If resolving children for a particular directory fails
                 log the error but carry on processing other directories
               */
              let%lwt children =
                if (isDirectory && nextDepth < maxDepth) {
                  attempt(() =>
                    getDirContent(~depth=nextDepth, path, getIcon)
                  )
                  |> Lwt.map(List.sort(sortByLoweredDisplayName));
                } else {
                  Lwt.return([]);
                };

              let parent =
                createFsNode(
                  ~path,
                  ~isDirectory,
                  ~depth=nextDepth,
                  ~displayName=file,
                  ~fileIcon=getIcon(path),
                );

              UiTree.{id: parentId, data: parent, isOpen: false, children}
              |> Lwt.return;
            },
            files,
          )
        )
    );
  };

  attempt(() => getDirContent(~depth=0, cwd, getIcon));
};

let getDirectoryTree = (cwd, languageInfo, iconTheme, ignored) => {
  let parentId = ExplorerId.getUniqueId();
  let getIcon = getFileIcon(languageInfo, iconTheme);
  let maxDepth = Constants.default.maximumExplorerDepth;
  let children =
    getFilesAndFolders(~maxDepth, ~ignored, cwd, getIcon)
    |> Lwt_main.run
    |> List.sort(sortByLoweredDisplayName);

  let parent =
    createFsNode(
      ~depth=0,
      ~path=cwd,
      ~displayName=Filename.basename(cwd),
      ~isDirectory=true,
      ~fileIcon=getIcon(cwd),
    );

  UiTree.{id: parentId, data: parent, isOpen: true, children};
};

let getNodePath = (node: UiTree.t) => node.data.path;

let getNodeId = (node: UiTree.t) => node.id;

let create = () => {directory: None, isOpen: true};

let reduce = (state: t, action: Actions.t) => {
  switch (action) {
  | SetExplorerTree(tree) => {directory: Some(tree), isOpen: true}
  | RemoveDockItem(WindowManager.ExplorerDock) => {...state, isOpen: false}
  | _ => state
  };
};
