open Oni_Core;
open Oni_Extensions;

module Log = (val Log.withNamespace("Oni2.Model.FileExplorer"));

type t = {
  tree: option(FsTreeNode.t),
  isOpen: bool,
  scrollOffset: [ | `Start(float) | `Middle(float) | `Reveal(int)],
  active: option(string), // path
  focus: option(string), // path
  decorations: StringMap.t(list(Decoration.t)),
};

[@deriving show({with_path: false})]
type action =
  | ActiveFilePathChanged(option(string))
  | TreeLoaded(FsTreeNode.t)
  | NodeLoaded(FsTreeNode.t)
  | FocusNodeLoaded(FsTreeNode.t)
  | NodeClicked(FsTreeNode.t)
  | ScrollOffsetChanged([ | `Start(float) | `Middle(float) | `Reveal(int)])
  | KeyboardInput(string);

let initial = {
  tree: None,
  isOpen: true,
  scrollOffset: `Start(0.),
  active: None,
  focus: None,
  decorations: StringMap.empty,
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

let isDirectory = path => Sys.file_exists(path) && Sys.is_directory(path);

let logUnixError = (error, fn, arg) =>
  Log.errorf(m => {
    let msg = Unix.error_message(error);
    m("%s encountered in %s called with %s", msg, fn, arg);
  });

let attempt = (~defaultValue, func) => {
  try%lwt(func()) {
  | Unix.Unix_error(error, fn, arg) =>
    logUnixError(error, fn, arg);
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
    let toFsTreeNode = file => {
      let path = Filename.concat(cwd, file);

      if (isDirectory(path)) {
        let%lwt children =
          if (loadChildren) {
            /**
               If resolving children for a particular directory fails
                log the error but carry on processing other directories
              */
            attempt(() => getDirContent(path), ~defaultValue=[])
            |> Lwt.map(List.sort(sortByLoweredDisplayName));
          } else {
            Lwt.return([]);
          };

        Lwt.return(
          FsTreeNode.directory(path, ~icon=getIcon(path), ~children),
        );
      } else {
        FsTreeNode.file(path, ~icon=getIcon(path)) |> Lwt.return;
      };
    };

    let%lwt files =
      Lwt_unix.files_of_directory(cwd)
      /* Filter out the relative name for current and parent directory*/
      |> Lwt_stream.filter(name => name != ".." && name != ".")
      /* Remove ignored files from search */
      |> Lwt_stream.filter(name => !List.mem(name, ignored))
      |> Lwt_stream.to_list;

    Lwt_list.map_p(toFsTreeNode, files);
  };

  attempt(() => getDirContent(cwd, ~loadChildren=true), ~defaultValue=[]);
};

let getDirectoryTree = (cwd, languageInfo, iconTheme, ignored) => {
  let getIcon = getFileIcon(languageInfo, iconTheme);
  let children =
    getFilesAndFolders(~ignored, cwd, getIcon)
    |> Lwt_main.run
    |> List.sort(sortByLoweredDisplayName);

  FsTreeNode.directory(cwd, ~icon=getIcon(cwd), ~children, ~isOpen=true);
};
