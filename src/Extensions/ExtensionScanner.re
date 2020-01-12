/*
 * ExtensionScanner.re
 *
 * Module to get and discover extension manifests
 */

open Oni_Core;
open Rench;

module Option = Oni_Core.Utility.Option;

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
  let items = Sys.readdir(directory) |> Array.to_list;

  let isDirectory = Sys.is_directory;

  let fullDirectory = d => Path.join(directory, d);
  let packageManifestPath = d => Path.join(d, "package.json");

  let loadPackageJson = pkg => {
    let json = Yojson.Safe.from_file(pkg);
    let path = Path.dirname(pkg);

    let localizationJsonPath = Path.join(path, "package.nls.json");

    let locDic = _getLocalizations(localizationJsonPath);
    Log.infof(m =>
      m(
        "Loaded %d localizations from %s",
        LocalizationDictionary.count(locDic),
        localizationJsonPath,
      )
    );

    try({
      let manifest =
        json
        |> ExtensionManifest.of_yojson_exn
        |> remapManifest(path)
        |> ExtensionManifest.updateName(prevName =>
             prefix
             |> Option.map(somePrefix => somePrefix ++ "." ++ prevName)
             |> Option.value(~default=prevName)
           )
        |> ExtensionManifest.localize(locDic);

      Some({category, manifest, path});
    }) {
    | ex =>
      Log.errorf(m =>
        m("Exception parsing %s : %s", pkg, Printexc.to_string(ex))
      );
      None;
    };
  };

  items
  |> List.map(fullDirectory)
  |> List.filter(isDirectory)
  |> List.map(packageManifestPath)
  |> List.filter(Sys.file_exists)
  |> List.map(loadPackageJson)
  |> Utility.List.filter_map(Utility.identity);
};
