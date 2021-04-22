open Oni_Core;
// MODEL

[@deriving show]
type command =
  | Reload;

[@deriving show]
type msg =
  | ActiveFilePathChanged([@opaque] option(FpExp.t(FpExp.absolute)))
  | Command(command)
  | NodeLoadError(string)
  | NodeLoaded(FsTreeNode.t)
  | FocusNodeLoaded(FsTreeNode.t)
  | FileWatcherEvent({
      path: [@opaque] FpExp.t(FpExp.absolute),
      event: Service_FileWatcher.event,
    })
  | Tree(Component_VimTree.msg);

module Msg = {
  let activeFileChanged = maybePath => ActiveFilePathChanged(maybePath);
};

type model = {
  fileWatcherKey: Service_FileWatcher.Key.t,
  rootPath: FpExp.t(FpExp.absolute),
  rootName: string,
  expandedPaths: list(FpExp.t(FpExp.absolute)),
  pathsToLoad: list(FpExp.t(FpExp.absolute)),
  tree: option(FsTreeNode.t),
  treeView: Component_VimTree.model(FsTreeNode.metadata, FsTreeNode.metadata),
  isOpen: bool,
  scrollOffset: [ | `Start(float) | `Middle(float) | `Reveal(int)],
  active: option(FpExp.t(FpExp.absolute)),
  focus: option(FpExp.t(FpExp.absolute)) // path
};

let initial = (~rootPath) => {
  {
    fileWatcherKey:
      Service_FileWatcher.Key.create(
        ~friendlyName="Explorer:" ++ FpExp.toString(rootPath),
      ),
    rootPath,
    rootName: "",
    expandedPaths: [rootPath],
    pathsToLoad: [rootPath],
    tree: None,
    treeView: Component_VimTree.create(~rowHeight=20),
    isOpen: true,
    scrollOffset: `Start(0.),
    active: None,
    focus: None,
  };
};

let setRoot = (~rootPath, model) => {
  ...model,
  rootPath,
  tree: None,
  active: None,
  focus: None,
  expandedPaths: [rootPath],
  pathsToLoad: [rootPath],
};

let root = ({rootPath, _}) => rootPath;

let getFileIcon = (~languageInfo, ~iconTheme, filePath) => {
  Exthost.LanguageInfo.getLanguageFromFilePath(languageInfo, filePath)
  |> Oni_Core.IconTheme.getIconForFile(iconTheme, filePath);
};

let getIndex = (path, model) => {
  Component_VimTree.(
    findIndex(
      fun
      | Node({data, _})
      | Leaf({data, _}) => FsTreeNode.(data.path) == path,
      model.treeView,
    )
  );
};

let getFocusedIndex = model => {
  model.active |> Utility.OptionEx.flatMap(path => getIndex(path, model));
};
