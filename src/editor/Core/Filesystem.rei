type t('a) =
  | Ok('a)
  | Error(string);

let copy: (string, string) => t(unit);

let isDir: string => t(unit);

let getOniHomeDir: unit => t(unit);

let createOniDirectory: unit => t(unit);
