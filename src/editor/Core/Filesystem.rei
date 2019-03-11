type t('a) =
  | Ok('a)
  | Error(string);

let copy: (string, string) => t(unit);

let isDir: Unix.stats => t(unit);

let getHomeDir: unit => t(string);

let createOniDirectory: unit => t(unit);
