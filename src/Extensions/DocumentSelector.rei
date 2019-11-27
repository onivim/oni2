/*
 * DocumentSelector.re
 *
 * This module documents & types the protocol for communicating with the VSCode Extension Host.
 */

type t;

let matches: (t, string) => bool;

let of_yojson: Yojson.Safe.t => result(t, string);
let to_yojson: t => Yojson.Safe.t;

/*
  [create(fileType)] creates a document selector that matches a single filetype
 */
let create: string => t;

let toString: t => string;
