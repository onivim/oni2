open Oni_Core;

module Log = (val Log.withNamespace("Oni2.Model.FileExplorer"));

type t = {
  tree: option(FsTreeNode.t),
  isOpen: bool,
  scrollOffset: [ | `Start(float) | `Middle(float) | `Reveal(int)],
  active: option(string), // path
  focus: option(string) // path
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
};

let getFileIcon = (languageInfo, iconTheme, filePath) => {
  let fileIcon =
    Exthost.LanguageInfo.getLanguageFromFilePath(languageInfo, filePath)
    |> IconTheme.getIconForFile(iconTheme, filePath);

  switch (fileIcon) {
  | Some(_) as x => x
  | None => None
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
  Luv.File.Dirent.(
    cwd
    |> Service_OS.Api.readdir
    |> Lwt.map((dirents: list(Luv.File.Dirent.t)) => {
         dirents
         |> List.filter(({name, _}) =>
              name != ".." && name != "." && !List.mem(name, ignored)
            )
         |> List.filter_map(({name, kind}) => {
              let path = Filename.concat(cwd, name);
              if (kind == `FILE || kind == `LINK) {
                Some(FsTreeNode.file(path, ~icon=getIcon(path)));
              } else if (kind == `DIR) {
                Some(
                  FsTreeNode.directory(
                    path,
                    ~icon=getIcon(path),
                    ~children=[],
                  ),
                );
              } else {
                None;
              };
            })
         |> List.sort(sortByLoweredDisplayName)
       })
  );
};

let getDirectoryTree = (cwd, languageInfo, iconTheme, ignored) => {
  let getIcon = getFileIcon(languageInfo, iconTheme);
  let childrenPromise = getFilesAndFolders(~ignored, cwd, getIcon);

  childrenPromise
  |> Lwt.map(children => {
       FsTreeNode.directory(cwd, ~icon=getIcon(cwd), ~children, ~isOpen=true)
     });
};
