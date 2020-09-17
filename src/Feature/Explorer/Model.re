// MODEL

[@deriving show]
type msg =
  | ActiveFilePathChanged(option(string))
  | TreeLoaded(FsTreeNode.t)
  | TreeLoadError(string)
  | NodeLoaded(FsTreeNode.t)
  | FocusNodeLoaded(FsTreeNode.t)
  | NodeClicked(FsTreeNode.t)
  | ScrollOffsetChanged([ | `Start(float) | `Middle(float) | `Reveal(int)])
  | KeyboardInput(string);

module Msg = {
  let keyPressed = key => KeyboardInput(key);
  let activeFileChanged = maybePath => ActiveFilePathChanged(maybePath);
};

type model = {
  rootPath: string,
  tree: option(FsTreeNode.t),
  isOpen: bool,
  scrollOffset: [ | `Start(float) | `Middle(float) | `Reveal(int)],
  active: option(string), // path
  focus: option(string) // path
};

let initial = (~rootPath) => {
  rootPath,
  tree: None,
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
