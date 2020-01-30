/*
 * ExtensionScanner.re
 *
 * Module to get and discover extension manifests
 */

open Oni_Core;
open Rench;

module Option = Utility.Option;
module OptionEx = Utility.OptionEx;
module Result = Utility.Result;

module Log = (val Log.withNamespace("Oni2.Extensions.ExtensionScanner"));

type category =
  | Default
  | Bundled
  | User
  | Development;

type t = {
  category,
  manifest: ExtensionManifest.t,
  path: string,
};

let remapManifest = (directory: string, manifest: ExtensionManifest.t) => {
  let manifest = {
    ...manifest,
    main: Option.map(m => Path.join(directory, m), manifest.main),
  };

  ExtensionManifest.remapPaths(directory, manifest);
};

let _getLocalizations = path =>
  if (Sys.file_exists(path)) {
    path |> Yojson.Safe.from_file |> LocalizationDictionary.of_yojson;
  } else {
    LocalizationDictionary.initial;
  };

let scan = (~prefix=None, ~category, directory: string) => {
  let loadManifest = pkg => {
    let json = Yojson.Safe.from_file(pkg);
    let directory = Path.dirname(pkg);
    let nlsPath = Path.join(directory, "package.nls.json");

    let localize = {
      let dict = _getLocalizations(nlsPath);

      Log.infof(m => {
        let count = LocalizationDictionary.count(dict);
        m("Loaded %d localizations from %s", count, nlsPath);
      });

      ExtensionManifest.localize(dict);
    };

    try({
      let manifest =
        json
        |> Json.Decode.decode_value(ExtensionManifest.decode)
        |> Result.map_error(Json.Decode.string_of_error)
        |> Result.get_ok
        |> remapManifest(directory)
        |> ExtensionManifest.updateName(name =>
             prefix
             |> Option.map(prefix => prefix ++ "." ++ name)
             |> Option.value(~default=name)
           )
        |> localize;

      Some({category, manifest, path: directory});
    }) {
    | ex =>
      Log.errorf(m =>
        m("Exception parsing %s : %s", pkg, Printexc.to_string(ex))
      );
      None;
    };
  };

  Sys.readdir(directory)
  |> Array.to_list
  |> List.map(Path.join(directory))
  |> List.filter(Sys.is_directory)
  |> List.map(dir => Path.join(dir, "package.json"))
  |> List.filter(Sys.file_exists)
  |> List.map(loadManifest)
  |> OptionEx.values;
};
