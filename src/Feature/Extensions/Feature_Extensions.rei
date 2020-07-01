open Oni_Core;
open Exthost.Extension;

type model;

[@deriving show({with_path: false})]
type msg =
  | Activated(string /* id */)
  | Discovered([@opaque] list(Scanner.ScanResult.t))
  | ExecuteCommand({
      command: string,
      arguments: [@opaque] list(Json.t),
    });

//let empty: t;

//let update: (msg, model) => model;
