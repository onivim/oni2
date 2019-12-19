/*
 * Extensions.re
 *
 * This module models state around loaded / activated extensions
 * for the 'Hover' view
 */
open Oni_Extensions;

type t = {
  activatedIds: list(string),
  extensions: list(ExtensionScanner.t),
};

[@deriving show({with_path: false})]
type action =
  | Activated(string /* id */)
  | Discovered([@opaque] list(ExtensionScanner.t));

let empty = {activatedIds: [], extensions: []};

let activateExtension = (id: string, v: t) => {
  ...v,
  activatedIds: [id, ...v.activatedIds],
};
