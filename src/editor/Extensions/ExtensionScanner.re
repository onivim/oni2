/*
 * ExtensionScanner.re
 *
 * Module to get and discover extension manifests
 */

open Rench;

type t = {
  manifest: ExtensionManifest.t,
  path: string,
};

let readFileSync = path => {
  let chan = open_in_bin(path);
  let data = ref("");
  try (
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

let scan = (directory: string) => {
  let items = Sys.readdir(directory) |> Array.to_list;

  let isDirectory = Sys.is_directory;

  let fullDirectory = d => Path.join(directory, d);
  let packageManifestPath = d => Path.join(d, "package.json");

  let loadPackageJson = pkg => {
    let json = readFileSync(pkg) |> Yojson.Safe.from_string;
    {
      manifest: ExtensionManifest.of_yojson_exn(json),
      path: Path.dirname(pkg),
    };
  };

  items
  |> List.map(fullDirectory)
  |> List.filter(isDirectory)
  |> List.map(packageManifestPath)
  |> List.filter(Sys.file_exists)
  |> List.map(loadPackageJson);
};
