/*
 * DocumentSelector.re
 *
 * This module documents & types the protocol for communicating with the VSCode Extension Host.
 */

[@deriving yojson({strict: false})]
type item = {language: string};

[@deriving yojson({strict: false})]
type t = list(item);

let matches = (v: t, fileType: string) => {
  List.exists(item => String.equal(item.language, fileType), v);
};

let toString = (v: t) => {
  "DocumentSelector("
  ++ (v |> List.map(x => x.language) |> String.concat(","))
  ++ ")";
};

let create = language => {
  [{language: language}];
};

let toString = (v: t) => {
  v
  |> List.map((item) => item.language)
  |> String.concat(", ");
};
