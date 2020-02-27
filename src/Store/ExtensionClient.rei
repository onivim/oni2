open Oni_Core;
open Oni_Model;
open Oni_Extensions;

let create:
  (~extensions: list(ExtensionScanner.t), ~setup: Setup.t) =>
  (ExtHostClient.t, Isolinear.Stream.t(Actions.t));
