// MODEL

[@deriving show]
type msg =
  | ActiveFilePathChanged(option(string))
  | TreeLoaded(FsTreeNode.t)
  | TreeLoadError(string)
  | NodeLoaded(FsTreeNode.t)
  | FocusNodeLoaded(FsTreeNode.t)
  | KeyboardInput(string)
  | Tree(Component_VimTree.msg);

module Msg = {
  let keyPressed = key => KeyboardInput(key);
  let activeFileChanged = maybePath => ActiveFilePathChanged(maybePath);
};

type model = {
  rootPath: string,
  rootName: string,
  tree: option(FsTreeNode.t),
  treeView: Component_VimTree.model(FsTreeNode.metadata, FsTreeNode.metadata),
  isOpen: bool,
  scrollOffset: [ | `Start(float) | `Middle(float) | `Reveal(int)],
  active: option(string), // path
  focus: option(string) // path
};

let initial = (~rootPath) => {
  rootPath,
  rootName: "",
  tree: None,
  treeView: Component_VimTree.create(~rowHeight=20),
  isOpen: true,
  scrollOffset: `Start(0.),
  active: None,
  focus: None,
};

let setRoot = (~rootPath, model) => {
  ...model,
  rootPath,
  tree: None,
  active: None,
  focus: None,
};

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
  model.active
  |> Oni_Core.Utility.OptionEx.flatMap(path => getIndex(path, model));
};
