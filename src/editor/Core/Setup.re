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
  fontsPath: [@default None] option(string),
  imagesPath: [@default None] option(string),
};

let version = "0.2.0";

let getFontPath = (setup: t, font) => {
  switch (setup.fontsPath) {
  | None => font
  | Some(v) => v ++ font;
  }
};

let getImagePath = (setup: t, image) => {
  switch (setup.imagesPath) {
  | None => image
  | Some(v) => v ++ image;
  }
}

let default = () => {
  let execDir = Revery.Environment.executingDirectory;

  switch (Revery.Environment.os) {
  | Revery.Environment.Windows => {
      nodePath: execDir ++ "node.exe",
      camomilePath: execDir ++ "camomile",
      textmateServicePath: execDir ++ "textmate_service/lib/src/index.js",
      bundledExtensionsPath: execDir ++ "extensions",
      developmentExtensionsPath: None,
      extensionHostPath: "",
      rgPath: execDir ++ "rg.exe",
      fontsPath: None,
      imagesPath: None,
      version,
    }
  | Revery.Environment.Mac => {
      nodePath: execDir ++ "node",
      camomilePath: execDir ++ "../Resources/camomile",
      textmateServicePath: execDir ++ "../Resources/textmate_service/lib/src/index.js",
      bundledExtensionsPath: execDir ++ "../Resources/extensions",
      developmentExtensionsPath: None,
      extensionHostPath: "",
      rgPath: execDir ++ "rg",
      fontsPath: Some(execDir ++ "../Resources/fonts"),
      imagesPath: Some(execDir ++ "../Resources/images"),
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
      fontsPath: None,
      imagesPath: None,
      version,
    }
  };
};

let ofString = str => Yojson.Safe.from_string(str) |> of_yojson_exn;

let ofFile = filePath => Yojson.Safe.from_file(filePath) |> of_yojson_exn;

let init = () => {
  let setupJsonPath = Revery.Environment.executingDirectory ++ "setup.json";

  Log.debug("Setup: Looking for setupJson at: " ++ setupJsonPath);

  if (Sys.file_exists(setupJsonPath)) {
    ofFile(setupJsonPath);
  } else {
    default();
  };
};
