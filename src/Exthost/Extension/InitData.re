open Oni_Core;

module Log = (val Timber.Log.withNamespace("Exthost.Extension.InitData"));

module Identifier = {
  [@deriving (show, yojson({strict: false}))]
  type t = {
    value: string,
    _lower: string,
  };

  let fromString = str => {value: str, _lower: String.lowercase_ascii(str)};
};

module Hacks = {
  let patchIonideDependencies = (dependencies: list(Yojson.Safe.t)) => {
    dependencies
    |> List.map(
         fun
         | `String(str) =>
           if (str == "ms-dotnettools.csharp") {
             `String("muhammad-sammy.csharp");
           } else {
             `String(str);
           }
         | json => json,
       );
  };

  // TEMPORARY workarounds for external bugs blocking extensions
  let hacks = [
    // Workaround for https://github.com/open-vsx/publish-extensions/issues/106
    (
      "matklad.rust-analyzer",
      Utility.JsonEx.update(
        "releaseTag",
        fun
        | Some(`String(str)) => Some(`String(str))
        | _ => Some(`String("2020-08-04")),
      ),
    ),
    (
      // Workaround for https://github.com/onivim/oni2/issues/2974
      "ionide.ionide-fsharp",
      Utility.JsonEx.update(
        "extensionDependencies",
        fun
        | Some(`List(dependencies)) =>
          Some(`List(dependencies |> patchIonideDependencies))
        | json => json,
      ),
    ),
  ];

  let apply = (~extensionId, initDataJson) => {
    hacks
    |> List.fold_left(
         (acc, curr) => {
           let (toApplyId, hackF) = curr;

           if (String.equal(toApplyId, extensionId)) {
             Log.infof(m => m("Applying extension patch to: %s", toApplyId));
             hackF(acc);
           } else {
             acc;
           };
         },
         initDataJson,
       );
  };
};

module Extension = {
  [@deriving (show, yojson({strict: false}))]
  type t = Yojson.Safe.t;

  let ofScanResult = (scanner: Scanner.ScanResult.t) => {
    let extensionId = scanner.manifest |> Manifest.identifier;
    let identifier = extensionId |> Identifier.fromString;
    let extensionLocation = scanner.path |> Uri.fromPath;

    let json =
      switch (scanner.rawPackageJson) {
      | `Assoc(items) =>
        `Assoc([
          ("identifier", Identifier.to_yojson(identifier)),
          ("extensionLocation", Uri.to_yojson(extensionLocation)),
          ...items,
        ])
      | json =>
        Log.warnf(m =>
          m("Unexpected package format: %s", Yojson.Safe.to_string(json))
        );
        json;
      };

    json |> Hacks.apply(~extensionId);
  };
};

module StaticWorkspaceData = {
  [@deriving (show, yojson({strict: false}))]
  type t = {
    id: string,
    name: string,
  };

  let global = {id: "global", name: "global"};
};

module Environment = {
  [@deriving (show, yojson({strict: false}))]
  type t = {
    isExtensionDevelopmentDebug: bool,
    appName: string,
    appLanguage: string,
    appRoot: Uri.t,
    globalStorageHome: option(Uri.t),
    workspaceStorageHome: option(Uri.t),
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
    // TODO: Set up correctly
    workspaceStorageHome:
      Oni_Core.Filesystem.getWorkspaceStorageFolder()
      |> Result.to_option
      |> Option.map(Uri.fromFilePath),
    globalStorageHome:
      Oni_Core.Filesystem.getGlobalStorageFolder()
      |> Result.to_option
      |> Option.map(Uri.fromFilePath),
    userHome:
      Oni_Core.Filesystem.getUserDataDirectory()
      |> Result.to_option
      |> Option.map(Uri.fromFilePath),
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
  workspace: StaticWorkspaceData.t,
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
      ~workspace=StaticWorkspaceData.global,
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
    workspace,
  };
};
