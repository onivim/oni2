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
  [@deriving show({with_path: false})]
  type t = {
    /* TODO:  */
    /* isBuiltIn: bool, */
    /* isUnderDevelopment: bool, */
    identifier: string,
    extensionLocationPath: string,
    name: string,
    /* displayName: string, */
    /* publisher: string, */
    main: option(string),
    version: string,
    engines: string,
    activationEvents: list(string),
    extensionDependencies: list(string),
    extensionKind: ExtensionManifest.kind,
    contributes: ExtensionContributions.t,
    enableProposedApi: bool,
  };

  let ofScannedExtension = (extensionInfo: ExtensionScanner.t) => {
    let {path, manifest, _} = extensionInfo;

    {
      identifier: manifest.name,
      extensionLocationPath: path,
      name: manifest.name,
      /* publisher: manifest.publisher, */
      main: manifest.main,
      version: manifest.version,
      engines: manifest.engines,
      activationEvents: manifest.activationEvents,
      extensionDependencies: manifest.extensionDependencies,
      extensionKind: manifest.extensionKind,
      contributes: manifest.contributes,
      enableProposedApi: manifest.enableProposedApi,
    };
  };

  let encode = data =>
    Json.Encode.(
      obj([
        ("identifier", data.identifier |> string),
        ("extensionLocationPath", data.extensionLocationPath |> string),
        ("name", data.name |> string),
        ("main", data.main |> option(string)),
        ("version", data.version |> string),
        ("engines", data.engines |> string),
        ("activationEvents", data.activationEvents |> list(string)),
        ("extensionDependencies", data.extensionDependencies |> list(string)),
        ("extensionKind", data.extensionKind |> ExtensionManifest.Encode.kind),
        ("contributes", data.contributes |> ExtensionContributions.encode),
      ])  
    );
};

module Workspace = {
  [@deriving show({with_path: false})]
  type t = {__test: string};

  let encode = workspace =>
    Json.Encode.(
      obj([
        ("__test", workspace.__test |> string)
      ])
    );
};

module Environment = {
  [@deriving show({with_path: false})]
  type t = {globalStorageHomePath: string};

  let create = (~globalStorageHomePath=Filesystem.unsafeFindHome(), ()) => {
    let ret: t = {globalStorageHomePath: globalStorageHomePath};
    ret;
  };

  let encode = env =>
    Json.Encode.(
      obj([
        ("globalStorageHomePath", env.globalStorageHomePath |> string)
      ])
    );
};

[@deriving show({with_path: false})]
type t = {
  extensions: list(ExtensionInfo.t),
  parentPid: int,
  environment: Environment.t,
  logsLocationPath: string,
  autoStart: bool,
  workspace: Workspace.t,
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
  workspace: {
    __test: "",
  },
};

let encode = data =>
  Json.Encode.(
    obj([
      ("extensions", data.extensions |> list(ExtensionInfo.encode)),
      ("parentPid", data.parentPid |> int),
      ("environment", data.environment |> Environment.encode),
      ("logsLocationPath", data.logsLocationPath |> string),
      ("autoStart", data.autoStart |> bool),
      ("workspace", data.workspace |> Workspace.encode),
    ])  
  );