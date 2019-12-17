/*
 * References.re
 *
 * Model for state
 */

module Ext = Oni_Extensions;

type t = list(Ext.LocationWithUri.t);
let initial: t = [];

[@deriving show({with_path: false})]
type actions =
  | Requested
  | Set([@opaque] list(Ext.LocationWithUri.t));
