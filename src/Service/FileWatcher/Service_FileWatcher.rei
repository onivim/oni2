open Oni_Core;

module Key: {
  type t;

  let create: (~friendlyName: string) => t;
};

[@deriving show]
type event = {
  watchedPath: FpExp.t(FpExp.absolute),
  changedPath: FpExp.t(FpExp.absolute),
  hasRenamed: bool,
  hasChanged: bool,
};

let watch:
  (~key: Key.t, ~path: FpExp.t(FpExp.absolute), ~onEvent: event => 'msg) =>
  Isolinear.Sub.t('msg);
