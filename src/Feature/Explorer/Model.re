open Oni_Core;

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
    let keyPressed = key => KeyboardInput(key)
};

type model = {
  tree: option(FsTreeNode.t),
  isOpen: bool,
  scrollOffset: [ | `Start(float) | `Middle(float) | `Reveal(int)],
  active: option(string), // path
  focus: option(string), // path
  decorations: StringMap.t(list(Decoration.t)),
}

let initial = {
  tree: None,
  isOpen: true,
  scrollOffset: `Start(0.),
  active: None,
  focus: None,
  decorations: StringMap.empty,
};
