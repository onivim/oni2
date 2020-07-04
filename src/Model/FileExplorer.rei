open Oni_Core;

type t = {
  tree: option(FsTreeNode.t),
  isOpen: bool,
  scrollOffset: [ | `Start(float) | `Middle(float) | `Reveal(int)],
  active: option(string), // path
  focus: option(string), // path
  decorations: StringMap.t(list(Decoration.t)),
};

[@deriving show]
type action =
  | ActiveFilePathChanged(option(string))
  | TreeLoaded(FsTreeNode.t)
  | NodeLoaded(FsTreeNode.t)
  | FocusNodeLoaded(FsTreeNode.t)
  | NodeClicked(FsTreeNode.t)
  | ScrollOffsetChanged([ | `Start(float) | `Middle(float) | `Reveal(int)])
  | KeyboardInput(string);

let initial: t;

let getFileIcon:
  (Exthost.LanguageInfo.t, IconTheme.t, string) =>
  option(IconTheme.IconDefinition.t);
let getDirectoryTree:
  (string, Exthost.LanguageInfo.t, IconTheme.t, list(string)) => FsTreeNode.t;
