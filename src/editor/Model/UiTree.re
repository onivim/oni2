module ReveryTree = Revery.UI.Components.Tree;

type fsNode('a) = {
  displayName: string,
  path: string,
  isDirectory: bool,
  children: list('a),
  icon: option(IconTheme.IconDefinition.t),
  secondaryIcon: option(IconTheme.IconDefinition.t),
};

type treeItem =
  | FileSystemNode(fsNode(treeItem));

type t = ReveryTree.tree(treeItem);
type content = ReveryTree.content(treeItem);

let empty = ReveryTree.Empty;

type treeUpdate = {
  updated: t,
  tree: t,
};
