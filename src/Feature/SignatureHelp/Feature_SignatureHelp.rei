open Oni_Core;
open EditorCoreTypes;

type model;

let initial: model;

[@deriving show({with_path: false})]
type command =
  | Show;

[@deriving show({with_path: false})]
type msg =
  | Command(command);

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg));
