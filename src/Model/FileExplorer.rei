type t = {
  tree: option(FsTreeNode.t),
  isOpen: bool,
  scrollOffset: float,
  focus: option(string) // path
};

[@deriving show]
type action =
  | TreeLoaded([@opaque] FsTreeNode.t)
  | NodeLoaded(int, [@opaque] FsTreeNode.t)
  | FocusNodeLoaded(int, [@opaque] FsTreeNode.t)
  | NodeClicked([@opaque] FsTreeNode.t)
  | ScrollOffsetChanged(float);

let initial: t;

let getFileIcon:
  (LanguageInfo.t, IconTheme.t, string) => option(IconTheme.IconDefinition.t);
let getDirectoryTree:
  (string, LanguageInfo.t, IconTheme.t, list(string)) => FsTreeNode.t;
