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
  ++ List.fold_left((prev, curr) => {prev ++ "," ++ curr.language}, "", v)
  ++ ")";
};

let create = language => {
  [{language: language}];
};
