/*
 * LocalizedToken.rei
 */

[@deriving show]
type t;

let parse: string => t;

/**
 [localize(token)] returns a new token that has been localized.
*/
let localize: (LocalizationDictionary.t, t) => t;

exception LocalizedTokenParseException;

let of_yojson: Yojson.Safe.t => result(t, string);
let of_yojson_exn: Yojson.Safe.t => t;
let to_yojson: t => Yojson.Safe.t;

/*
  [to_string(token)] returns a string representation.

  If the token has been localized with [localize], and has a localization,
  the localized token will be returned.

  Otherwise, the raw token will be returned.
 */
let to_string: t => string;
