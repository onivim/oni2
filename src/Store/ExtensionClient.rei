open Oni_Core;
open Oni_Model;
open Oni_Extensions;
open Exthost;
open Exthost.Extension;

let create:
  (
    ~config: Feature_Configuration.model,
    ~extensions: list(Scanner.ScanResult.t),
    ~setup: Setup.t
  ) =>
  (result(Exthost.Client.t, string), Isolinear.Stream.t(Actions.t));
