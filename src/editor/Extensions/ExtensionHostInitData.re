/*
 * ExtensionHostInitData.re
 *
 * Module documenting the initialization data required for spinning up
 * and activating the extension host
 */

open Rench;

open Oni_Core;
open ExtensionManifest;
open ExtensionScanner;

/*
 * ExtensionInfo
 *
 * This is the type we pass to the extension host.
 * Must be kept in sync with the `IRawExtensionDescription` defined in extHost.protocol.ts:
 * https://github.com/onivim/vscode/blob/051b81fd1fa4fb656a5e1dab7235ac62dea58cfd/src/vs/workbench/api/node/extHost.protocol.ts#L79
 */
module ExtensionInfo = {
  [@deriving (show({with_path: false}), yojson({strict: false, exn: true}))]
  type t = {
    /* TODO:  */
    /* isBuiltIn: bool, */
    /* isUnderDevelopment: bool, */
    /* enableProposedApi: bool */
    identifier: string,
    extensionLocationPath: string,
    name: string,
    /* displayName: string, */
    /* publisher: string, */
    version: string,
    engines: Engine.t,
    activationEvents: list(string),
    extensionDependencies: list(string),
    extensionKind: ExtensionKind.t,
    contributes: ExtensionContributions.t,
  };

  let ofScannedExtension = (extensionInfo: ExtensionScanner.t) => {
    let {path, manifest} = extensionInfo;

    {
      identifier: manifest.name,
      extensionLocationPath: path,
      name: manifest.name,
      /* publisher: manifest.publisher, */
      version: manifest.version,
      engines: manifest.engines,
      activationEvents: manifest.activationEvents,
      extensionDependencies: manifest.extensionDependencies,
      extensionKind: manifest.extensionKind,
      contributes: manifest.contributes,
    };
  };
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
      ~parentPid=Process.pid(),
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
