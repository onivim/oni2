open Oni_Extensions;

type t = {
  tree: option(FsTreeNode.t),
  isOpen: bool,
  scrollOffset: [ | `Start(float) | `Middle(float)],
  active: option(string), // path
  focus: option(int), // node id
};

[@deriving show]
type action =
  | TreeLoaded([@opaque] FsTreeNode.t)
  | NodeLoaded(int, [@opaque] FsTreeNode.t)
  | FocusNodeLoaded(int, [@opaque] FsTreeNode.t)
  | NodeClicked([@opaque] FsTreeNode.t)
  | ScrollOffsetChanged([ | `Start(float) | `Middle(float)]);

let initial: t;

let getFileIcon:
  (LanguageInfo.t, IconTheme.t, string) => option(IconTheme.IconDefinition.t);
let getDirectoryTree:
  (string, LanguageInfo.t, IconTheme.t, list(string)) => FsTreeNode.t;
