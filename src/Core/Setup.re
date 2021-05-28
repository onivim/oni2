/*
 * Setup.re
 *
 * Runtime configuration of dependencies
 */
open Kernel;

module Log = (val Log.withNamespace("Oni2.Core.Setup"));

[@deriving (show({with_path: false}), yojson({strict: false, exn: true}))]
type t = {
  [@key "node"]
  nodePath: string,
  [@key "bundledExtensions"]
  bundledExtensionsPath: string,
  [@key "developmentExtensions"]
  developmentExtensionsPath: [@default None] option(string),
  [@key "nodeScript"]
  nodeScriptPath: string,
  [@key "rg"]
  rgPath: string,
};

let default = () => {
  let execDir = Revery.Environment.executingDirectory;

  switch (Revery.Environment.os) {
  | Windows(_) => {
      nodePath: execDir ++ "node.exe",
      nodeScriptPath: execDir ++ "node",
      bundledExtensionsPath: execDir ++ "extensions",
      developmentExtensionsPath: None,
      rgPath: execDir ++ "rg.exe",
    }
  | Mac(_) => {
      nodePath: execDir ++ "node",
      nodeScriptPath: execDir ++ "../Resources/node",
      bundledExtensionsPath: execDir ++ "../Resources/extensions",
      developmentExtensionsPath: None,
      rgPath: execDir ++ "rg",
    }
  | _ => {
      nodePath: execDir ++ "node",
      nodeScriptPath: execDir ++ "../share/node",
      bundledExtensionsPath: execDir ++ "extensions",
      developmentExtensionsPath: None,
      rgPath: execDir ++ "rg",
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
  switch (Sys.getenv_opt("ONI2_EXTHOST")) {
  | Some(extHostPath) =>
    Rench.Path.join(extHostPath, "out/bootstrap-fork.js")
  | None =>
    getNodeScriptPath(
      ~script="node_modules/@onivim/vscode-exthost/out/bootstrap-fork.js",
      v,
    )
  };
};

let init = () => {
  let setupJsonPath = Revery.Environment.executingDirectory ++ "setup.json";

  Log.debug("Looking for setup configuration at: " ++ setupJsonPath);

  if (Sys.file_exists(setupJsonPath)) {
    ofFile(setupJsonPath);
  } else {
    default();
  };
};
