/*
 * LocalizationDictionary.rei
 */

type t;

let initial: t;

let of_yojson: Yojson.Safe.t => t;

let get: (string, t) => option(string);

let count: t => int;
