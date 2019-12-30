open Oni_Extensions;

type t = {
  tree: option(FsTreeNode.t),
  isOpen: bool,
  scrollOffset: [ | `Start(float) | `Middle(float) | `Reveal(int)],
  active: option(string), // path
  focus: option(string) // path
};

[@deriving show]
type action =
  | TreeLoaded([@opaque] FsTreeNode.t)
  | NodeLoaded(string, [@opaque] FsTreeNode.t)
  | FocusNodeLoaded(string, [@opaque] FsTreeNode.t)
  | NodeClicked([@opaque] FsTreeNode.t)
  | ScrollOffsetChanged([ | `Start(float) | `Middle(float) | `Reveal(int)])
  | KeyboardInput(string);

let initial: t;

let getFileIcon:
  (LanguageInfo.t, IconTheme.t, string) => option(IconTheme.IconDefinition.t);
let getDirectoryTree:
  (string, LanguageInfo.t, IconTheme.t, list(string)) => FsTreeNode.t;
