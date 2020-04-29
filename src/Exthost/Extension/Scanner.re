/*
 * Scanner.re
 *
 * Module to get and discover extension manifests
 */

open Rench;
open Oni_Core;

module Log = (val Timber.Log.withNamespace("Exthost.Extension.Scanner"));

type category =
  | Default
  | Bundled
  | User
  | Development;

module ScanResult = {
  type t = {
    category,
    manifest: Manifest.t,
    path: string,
  };
};

let remapManifest = (directory: string, manifest: Manifest.t) => {
  Manifest.remapPaths(directory, manifest);
};

let _getLocalizations = path =>
  if (Sys.file_exists(path)) {
    path |> Yojson.Safe.from_file |> LocalizationDictionary.of_yojson;
  } else {
    LocalizationDictionary.initial;
  };

let load = (~prefix=None, ~category, packageFile) => {
  let json = Yojson.Safe.from_file(packageFile);
  let directory = Filename.dirname(packageFile);
  let nlsPath = Path.join(directory, "package.nls.json");

  let localize = {
    let dict = _getLocalizations(nlsPath);

    Log.infof(m => {
      let count = LocalizationDictionary.count(dict);
      m("Loaded %d localizations from %s", count, nlsPath);
    });

    Manifest.localize(dict);
  };

  switch (Json.Decode.decode_value(Manifest.decode, json)) {
  | Ok(parsedManifest) =>
    let manifest =
      parsedManifest
      |> remapManifest(directory)
      |> Manifest.updateName(name =>
           prefix
           |> Option.map(prefix => prefix ++ "." ++ name)
           |> Option.value(~default=name)
         )
      |> localize;

    Some(ScanResult.{category, manifest, path: directory});

  | Error(err) =>
    Log.errorf(m =>
      m(
        "Failed to parse %s:\n\t%s",
        packageFile,
        Json.Decode.string_of_error(err),
      )
    );
    None;
  };
};

let scan = (~prefix=None, ~category, directory: string) => {
  Sys.readdir(directory)
  |> Array.to_list
  |> List.map(Path.join(directory))
  |> List.filter(Sys.is_directory)
  |> List.map(dir => Path.join(dir, "package.json"))
  |> List.filter(Sys.file_exists)
  |> List.map(load(~category, ~prefix))
  |> List.filter_map(Fun.id);
};
