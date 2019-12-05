/*
 * Setup.re
 *
 * Runtime configuration of dependencies
 */

[@deriving (show({with_path: false}), yojson({strict: false, exn: true}))]
type t = {
  [@key "node"]
  nodePath: string,
  /* Camomile runtime files */
  [@key "camomile"]
  camomilePath: string,
  [@key "bundledExtensions"]
  bundledExtensionsPath: string,
  [@key "developmentExtensions"]
  developmentExtensionsPath: [@default None] option(string),
  [@key "nodeScript"]
  nodeScriptPath: string,
  [@key "rg"]
  rgPath: string,
  [@key "rls"]
  rlsPath: string,
  version: [@default "Unknown"] string,
};

let version = "0.2.0";

let default = () => {
  let execDir = Revery.Environment.executingDirectory;

  switch (Revery.Environment.os) {
  | Revery.Environment.Windows => {
      nodePath: execDir ++ "node.exe",
      nodeScriptPath: execDir ++ "node",
      camomilePath: execDir ++ "camomile",
      bundledExtensionsPath: execDir ++ "extensions",
      developmentExtensionsPath: None,
      rgPath: execDir ++ "rg.exe",
      rlsPath: execDir ++ "rls.exe",
      version,
    }
  | Revery.Environment.Mac => {
      nodePath: execDir ++ "node",
      nodeScriptPath: execDir ++ "../Resources/node",
      camomilePath: execDir ++ "../Resources/camomile",
      bundledExtensionsPath: execDir ++ "../Resources/extensions",
      developmentExtensionsPath: None,
      rgPath: execDir ++ "rg",
      rlsPath: execDir ++ "rls",
      version,
    }
  | _ => {
      nodePath: execDir ++ "node",
      nodeScriptPath: execDir ++ "../share/node",
      camomilePath: execDir ++ "../share/camomile",
      bundledExtensionsPath: execDir ++ "extensions",
      developmentExtensionsPath: None,
      rgPath: execDir ++ "rg",
      rlsPath: execDir ++ "rls",
      version,
    }
  };
};

let ofString = str => Yojson.Safe.from_string(str) |> of_yojson_exn;

let ofFile = filePath => Yojson.Safe.from_file(filePath) |> of_yojson_exn;

let getNodeHealthCheckPath = (v: t) => {
  v.nodeScriptPath ++ "/check-health.js";
};

let getNodeExtensionHostPath = (v: t) => {
  v.nodeScriptPath ++ "/node_modules/vscode-exthost/out/bootstrap-fork.js";
};

let init = () => {
  let setupJsonPath = Revery.Environment.executingDirectory ++ "setup.json";

  Log.debug(() => "Setup: Looking for setupJson at: " ++ setupJsonPath);

  if (Sys.file_exists(setupJsonPath)) {
    ofFile(setupJsonPath);
  } else {
    default();
  };
};
