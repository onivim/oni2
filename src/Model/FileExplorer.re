open Revery;
open Oni_Core;

type t = {
  tree: option(FsTreeNode.t),
  isOpen: bool,
};

[@deriving show({with_path: false})]
type action =
  | TreeUpdated([@opaque] FsTreeNode.t)
  | NodeUpdated(int, [@opaque] FsTreeNode.t)
  | NodeClicked([@opaque] FsTreeNode.t);

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

let sortByLoweredDisplayName = (a: FsTreeNode.t, b: FsTreeNode.t) => {
  switch (a.kind, b.kind) {
  | (Directory(_), File) => (-1)
  | (File, Directory(_)) => 1
  | _ =>
    compare(
      a.displayName |> String.lowercase_ascii,
      b.displayName |> String.lowercase_ascii,
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
let getFilesAndFolders = (~ignored, cwd, getIcon) => {
  let rec getDirContent = (~loadChildren=false, cwd) => {
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
              let id = ExplorerId.getUniqueId();

              /**
                 If resolving children for a particular directory fails
                 log the error but carry on processing other directories
               */
              let%lwt kind =
                if (isDirectory) {
                  let%lwt children =
                    if (loadChildren) {
                      let%lwt children =
                        handleError(~defaultValue=[], () =>
                          getDirContent(path)
                        );

                      children
                      |> List.sort(sortByLoweredDisplayName)
                      |> (children => `Loaded(children) |> Lwt.return);
                    } else {
                      Lwt.return(`Loading);
                    };

                  Lwt.return(
                    FsTreeNode.Directory({isOpen: false, children}),
                  );
                } else {
                  Lwt.return(FsTreeNode.File);
                };

              Lwt.return(
                FsTreeNode.create(~id, ~path, ~icon=getIcon(path), ~kind),
              );
            },
            files,
          )
        )
    );
  };

  handleError(~defaultValue=[], () => getDirContent(cwd, ~loadChildren=true));
};

let getDirectoryTree = (cwd, languageInfo, iconTheme, ignored) => {
  let id = ExplorerId.getUniqueId();
  let getIcon = getFileIcon(languageInfo, iconTheme);
  let children =
    getFilesAndFolders(~ignored, cwd, getIcon)
    |> Lwt_main.run
    |> List.sort(sortByLoweredDisplayName);

  FsTreeNode.create(
    ~id,
    ~path=cwd,
    ~icon=getIcon(cwd),
    ~kind=Directory({isOpen: true, children: `Loaded(children)}),
  );
};

let initial = {tree: None, isOpen: true};
