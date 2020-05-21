/*
 * References.re
 *
 * Model for state
 */

type t = list(Exthost.Location.t);
let initial: t = [];

[@deriving show({with_path: false})]
type actions =
  | Requested
  | Set([@opaque] list(Exthost.Location.t));
