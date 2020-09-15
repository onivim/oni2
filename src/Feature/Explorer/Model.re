// MODEL

[@deriving show]
type msg =
  | ActiveFilePathChanged(option(string))
  | TreeLoaded(FsTreeNode.t)
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
  tree: option(FsTreeNode.t),
  isOpen: bool,
  scrollOffset: [ | `Start(float) | `Middle(float) | `Reveal(int)],
  active: option(string), // path
  focus: option(string) // path
};

let initial = {
  tree: None,
  isOpen: true,
  scrollOffset: `Start(0.),
  active: None,
  focus: None,
};
