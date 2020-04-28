open Oni_Core;
open Oni_Model;
open Oni_Extensions;

let create:
  (
    ~config: Feature_Configuration.model,
    ~extensions: list(Exthost.Extension.Scanner.ScanResult.t),
    ~setup: Setup.t
  ) =>
  (ExtHostClient.t, Isolinear.Stream.t(Actions.t));
