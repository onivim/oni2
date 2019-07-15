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
  [@key "camomile"],
  camomilePath: string
  [@key "textmateService"]
  textmateServicePath: string,
  [@key "bundledExtensions"]
  bundledExtensionsPath: string,
  [@key "developmentExtensions"]
  developmentExtensionsPath: [@default None] option(string),
  [@key "extensionHost"]
  extensionHostPath: string,
  [@key "rg"]
  rgPath: string,
  version: [@default "Unknown"] string,
};

let version = "0.2.0";

let default = () => {
  let execDir = Utility.executingDirectory;

  switch (Revery.Environment.os) {
  | Revery.Environment.Windows => {
      nodePath: execDir ++ "node.exe",
      camomilePath: execDir ++ "camomile",
      textmateServicePath: execDir ++ "textmate_service/lib/src/index.js",
      bundledExtensionsPath: execDir ++ "extensions",
      developmentExtensionsPath: None,
      extensionHostPath: "",
      rgPath: execDir ++ "rg.exe",
      version,
    }
  | Revery.Environment.Mac => {
      nodePath: execDir ++ "node",
      camomilePath: execDir ++ "../camomile",
      textmateServicePath: execDir ++ "../textmate_service/lib/src/index.js",
      bundledExtensionsPath: execDir ++ "../extensions",
      developmentExtensionsPath: None,
      extensionHostPath: "",
      rgPath: execDir ++ "rg",
      version,
    }
  | _ => {
      nodePath: execDir ++ "node",
      camomilePath: execDir ++ "../share/camomile",
      textmateServicePath: execDir ++ "textmate_service/lib/src/index.js",
      bundledExtensionsPath: execDir ++ "extensions",
      developmentExtensionsPath: None,
      extensionHostPath: "",
      rgPath: execDir ++ "rg",
      version,
    }
  };
};

let ofString = str => Yojson.Safe.from_string(str) |> of_yojson_exn;

let ofFile = filePath => Yojson.Safe.from_file(filePath) |> of_yojson_exn;

let init = () => {
  let setupJsonPath = Utility.executingDirectory ++ "setup.json";

  Log.debug("Setup: Looking for setupJson at: " ++ setupJsonPath);

  if (Sys.file_exists(setupJsonPath)) {
    ofFile(setupJsonPath);
  } else {
    default();
  };
};
