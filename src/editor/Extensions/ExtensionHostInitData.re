/*
 * ExtensionHostInitData.re
 *
 * Module documenting the initialization data required for spinning up
 * and activating the extension host
 */

open Oni_Core;

module ExtensionInfo = {
  [@deriving (show({with_path: false}), yojson({strict: false, exn: true}))]
  type t = {identifier: string};
};

module Workspace = {
  [@deriving (show({with_path: false}), yojson({strict: false, exn: true}))]
  type t = {__test: string};
};

module Environment = {
  [@deriving (show({with_path: false}), yojson({strict: false, exn: true}))]
  type t = {globalStorageHomePath: string};

  let create = (~globalStorageHomePath=Filesystem.unsafeFindHome(), ()) => {
    let ret: t = {globalStorageHomePath: globalStorageHomePath};
    ret;
  };
};

[@deriving (show({with_path: false}), yojson({strict: false, exn: true}))]
type t = {
  extensions: list(ExtensionInfo.t),
  parentPid: int,
  environment: Environment.t,
  logsLocationPath: string,
  autoStart: bool,
};

let create =
    (
      ~parentPid=Unix.getpid(),
      ~extensions=[],
      ~environment=Environment.create(),
      ~logsLocationPath=Filesystem.unsafeFindHome(),
      ~autoStart=true,
      (),
    ) => {
  parentPid,
  extensions,
  environment,
  logsLocationPath,
  autoStart,
};
