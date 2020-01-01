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

let version = "0.3.0";

let default = () => {
  let execDir = Revery.Environment.executingDirectory;

  let defaultCamomilePath =
    switch (Revery.Environment.os) {
    | Revery.Environment.Windows => execDir ++ "camomile"
    | Revery.Environment.Mac => execDir ++ "../Resources/camomile"
    | _ => execDir ++ "../share/camomile"
    };

  let camomilePath =
    switch (Sys.getenv_opt(EnvironmentVariables.camomilePath)) {
    | None => defaultCamomilePath
    | Some(v) => v
    };

  switch (Revery.Environment.os) {
  | Revery.Environment.Windows => {
      nodePath: execDir ++ "node.exe",
      nodeScriptPath: execDir ++ "node",
      bundledExtensionsPath: execDir ++ "extensions",
      camomilePath,
      developmentExtensionsPath: None,
      rgPath: execDir ++ "rg.exe",
      rlsPath: execDir ++ "rls.exe",
      version,
    }
  | Revery.Environment.Mac => {
      nodePath: execDir ++ "node",
      nodeScriptPath: execDir ++ "../Resources/node",
      camomilePath,
      bundledExtensionsPath: execDir ++ "../Resources/extensions",
      developmentExtensionsPath: None,
      rgPath: execDir ++ "rg",
      rlsPath: execDir ++ "rls",
      version,
    }
  | _ => {
      nodePath: execDir ++ "node",
      nodeScriptPath: execDir ++ "../share/node",
      camomilePath,
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

let getNodeScriptPath = (~script: string, v: t) => {
  v.nodeScriptPath ++ "/" ++ script;
};

let getNodeHealthCheckPath = (v: t) => {
  getNodeScriptPath(~script="check-health.js", v);
};

let getNodeExtensionHostPath = (v: t) => {
  getNodeScriptPath(
    ~script="node_modules/vscode-exthost/out/bootstrap-fork.js",
    v,
  );
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
