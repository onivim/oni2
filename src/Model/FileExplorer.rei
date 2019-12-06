type t = {
  tree: option(FsTreeNode.t),
  isOpen: bool,
  focus: option(string) // path
};

[@deriving show]
type action =
  | TreeUpdated(FsTreeNode.t)
  | NodeUpdated(int, FsTreeNode.t)
  | NodeClicked(FsTreeNode.t);

let initial: t;

let getFileIcon:
  (LanguageInfo.t, IconTheme.t, string) => option(IconTheme.IconDefinition.t);
let getDirectoryTree:
  (string, LanguageInfo.t, IconTheme.t, list(string)) => FsTreeNode.t;
