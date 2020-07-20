open Oni_Core;

module Identifier = {
  [@deriving (show, yojson({strict: false}))]
  type t = {
    value: string,
    _lower: string,
  };

  let fromString = str => {value: str, _lower: String.lowercase_ascii(str)};
};

module Extension = {
  [@deriving (show, yojson({strict: false}))]
  type t = {
    identifier: Identifier.t,
    extensionLocation: Uri.t,
    name: string,
    displayName: option(string),
    description: option(string),
    main: option(string),
    icon: option(string),
    version: string,
    engines: string,
    activationEvents: list(string),
    extensionDependencies: list(string),
    extensionKind: list(string),
    contributes: Contributions.t,
    enableProposedApi: bool,
  };

  let ofManifestAndPath = (manifest: Manifest.t, path: string) => {
    identifier: manifest |> Manifest.identifier |> Identifier.fromString,
    extensionLocation: path |> Uri.fromPath,
    displayName: manifest |> Manifest.displayName,
    description: manifest.description,
    icon: manifest.icon,
    name: manifest.name,
    main: manifest.main,
    version: manifest.version,
    engines: manifest.engines,
    activationEvents: manifest.activationEvents,
    extensionDependencies: manifest.extensionDependencies,
    extensionKind: manifest.extensionKind |> List.map(Manifest.Kind.toString),
    enableProposedApi: manifest.enableProposedApi,
    contributes: manifest.contributes,
  };
};

module Environment = {
  [@deriving (show, yojson({strict: false}))]
  type t = {
    isExtensionDevelopmentDebug: bool,
    appName: string,
    appLanguage: string,
    appRoot: Uri.t,
    globalStorageHome: option(Uri.t),
    userHome: option(Uri.t),
    // TODO
    /*
     appUriScheme: string,
     appSettingsHome: option(Uri.t),
     webviewResourceRoot: string,
     webviewCspSource: string,
     useHostProxy: boolean,
     */
  };

  let default = () => {
    isExtensionDevelopmentDebug: false,
    appName: "Onivim 2",
    // TODO - INTL: Get proper user language
    appLanguage: "en-US",
    appRoot: Revery.Environment.getExecutingDirectory() |> Uri.fromPath,
    globalStorageHome:
      Oni_Core.Filesystem.getGlobalStorageFolder()
      |> Result.to_option
      |> Option.map(Uri.fromPath),
    userHome:
      Oni_Core.Filesystem.getUserDataDirectory()
      |> Result.to_option
      |> Option.map(Uri.fromPath),
  };
};

[@deriving (show, yojson({strict: false}))]
module Remote = {
  [@deriving (show, yojson({strict: false}))]
  type t = {
    isRemote: bool,
    // TODO:
    // authority: string,
  };

  let default = {isRemote: false};
};

[@deriving (show, yojson({strict: false}))]
module TelemetryInfo = {
  [@deriving (show, yojson({strict: false}))]
  type t = {
    sessionId: string,
    machineId: string,
    instanceId: string,
    msftInternal: bool,
  };

  let default = {
    sessionId: "Anonymous",
    machineId: "Anonymous",
    instanceId: "Anonymous",
    msftInternal: false,
  };
};

[@deriving (show, yojson({strict: false}))]
type t = {
  version: string,
  parentPid: int,
  extensions: list(Extension.t),
  resolvedExtensions: list(unit),
  hostExtensions: list(unit),
  environment: Environment.t,
  logLevel: int,
  logsLocation: Uri.t,
  logFile: Uri.t,
  autoStart: bool,
  remote: Remote.t,
  telemetryInfo: TelemetryInfo.t,
};

let create =
    (
      ~version,
      ~parentPid,
      ~logsLocation,
      ~logFile,
      ~environment=?,
      ~logLevel=0,
      ~autoStart=true,
      ~remote=Remote.default,
      ~telemetryInfo=TelemetryInfo.default,
      extensions,
    ) => {
  let environment =
    switch (environment) {
    | None => Environment.default()
    | Some(env) => env
    };
  {
    version,
    parentPid,
    logLevel,
    extensions,
    resolvedExtensions: [],
    hostExtensions: [],
    environment,
    logsLocation,
    logFile,
    autoStart,
    remote,
    telemetryInfo,
  };
};
