open Revery;
open Oni_Core;

type t = {directory: UiTree.t};

module ExplorerId =
  UniqueId.Make({});

let toFsNode = node =>
  switch (node) {
  | UiTree.FileSystemNode(n) => n
  };

let getFileIcon = (languageInfo, iconTheme, filePath) => {
  let fileIcon =
    LanguageInfo.getLanguageFromFilePath(languageInfo, filePath)
    |> IconTheme.getIconForFile(iconTheme, filePath);

  switch (fileIcon) {
  | Some(icon) as x => x
  | None => None
  };
};

let rec listToTree = (nodes, parent) => {
  open UI.Components.Tree;

  let parentId = ExplorerId.getUniqueId();
  let children =
    List.map(
      node => {
        let fsNode = toFsNode(node);
        let descendantNodes = List.map(toFsNode, fsNode.children);

        let descendants =
          UiTree.(
            List.map(
              c => listToTree(c.children, FileSystemNode(c)),
              descendantNodes,
            )
          );

        let id = ExplorerId.getUniqueId();
        Node({id, data: node, status: Closed}, descendants);
      },
      nodes,
    );

  Node({id: parentId, data: parent, status: Open}, children);
};

let toIcon = (~character, ~color) =>
  IconTheme.IconDefinition.{fontCharacter: character, fontColor: color};

let createFsNode = (~children, ~path, ~displayName, ~fileIcon, ~isDirectory) => {
  let createIcon = toIcon(~color=Revery.Colors.white);
  let (primary, secondary) =
    isDirectory
      ? (
        Some(createIcon(~character=FontAwesome.folder)),
        Some(createIcon(~character=FontAwesome.folderOpen)),
      )
      : (fileIcon, None);

  UiTree.FileSystemNode({
    path,
    displayName,
    children,
    isDirectory,
    icon: primary,
    secondaryIcon: secondary,
  });
};

let create = () => {directory: UiTree.empty};

let reduce = (state: t, action: Actions.t) => {
  switch (action) {
  | SetExplorerTree(tree) => {directory: tree}
  | _ => state
  };
};
