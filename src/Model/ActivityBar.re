[@deriving show({with_path: false})]
type action =
  | FileExplorerClick
  | SearchClick
  | SCMClick
  | ExtensionsClick;
