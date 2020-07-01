open Oni_Core;
open Exthost.Extension;

[@deriving show({with_path: false})]
type msg =
  | Activated(string /* id */)
  | Discovered([@opaque] list(Scanner.ScanResult.t))
  | ExecuteCommand({
      command: string,
      arguments: [@opaque] list(Json.t),
    });

type model = unit;
