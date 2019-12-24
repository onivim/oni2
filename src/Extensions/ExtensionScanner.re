/*
 * ExtensionScanner.re
 *
 * Module to get and discover extension manifests
 */

open Rench;

module Option = Oni_Core.Utility.Option;

module Log = (val Oni_Core.Log.withNamespace("Oni2.ExtensionScanner"));

type t = {
  manifest: ExtensionManifest.t,
  path: string,
};

let readFileSync = path => {
  let chan = open_in_bin(path);
  let data = ref("");
  try(
    {
      while (true) {
        data := data^ ++ "\n" ++ input_line(chan);
      };
      data^;
    }
  ) {
  | End_of_file =>
    close_in(chan);
    data^;
  };
};

let remapManifest = (directory: string, manifest: ExtensionManifest.t) => {
  let m =
    switch (manifest.main) {
    | None => manifest
    | Some(v) => {...manifest, main: Some(Path.join(directory, v))}
    };

  ExtensionManifest.remapPaths(directory, m);
};

let _getLocalizations = path =>
  if (Sys.file_exists(path)) {
    path |> Yojson.Safe.from_file |> LocalizationDictionary.of_yojson;
  } else {
    LocalizationDictionary.initial;
  };

let scan = (~prefix=None, directory: string) => {
  let items = Sys.readdir(directory) |> Array.to_list;

  let isDirectory = Sys.is_directory;

  let fullDirectory = d => Path.join(directory, d);
  let packageManifestPath = d => Path.join(d, "package.json");

  let loadPackageJson = pkg => {
    let json = readFileSync(pkg) |> Yojson.Safe.from_string;
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

    {manifest, path};
  };

  items
  |> List.map(fullDirectory)
  |> List.filter(isDirectory)
  |> List.map(packageManifestPath)
  |> List.filter(Sys.file_exists)
  |> List.map(loadPackageJson);
};
