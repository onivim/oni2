open Revery;
open Oni_Core;

[@deriving show({with_path: false})]
type action =
  | FileExplorerClick
  | SearchClick
  | ExtensionsClick;
