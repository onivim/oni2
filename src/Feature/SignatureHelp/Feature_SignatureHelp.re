/* Feature_Hover.re
   This feature project contains logic related to Signature Help
    */
open Oni_Core;
open Revery;
open Revery.UI;
open Revery.UI.Components;
open EditorCoreTypes;

module Log = (val Log.withNamespace("Oni.Feature.SignatureHelp"));

[@deriving show({with_path: false})]
type provider = {
  handle: int,
  selector: list(Exthost.DocumentFilter.t),
  metadata: Exthost.SignatureHelp.ProviderMetadata.t,
};

type model = {
  shown: bool,
  providers: list(provider),
};

let initial = {shown: false, providers: []};

[@deriving show({with_path: false})]
type command =
  | Show;

[@deriving show({with_path: false})]
type msg =
  | Command(command);

type outmsg =
  | Nothing
  | Effect(Isolinear.Effect.t(msg));
