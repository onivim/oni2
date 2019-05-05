include Revery.UI.Components.Tree;

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

type t = tree(treeItem);
type itemContent = content(treeItem);

type treeUpdate = {
  updated: t,
  tree: t,
};
