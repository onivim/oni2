module Scheme = {
  type t = string;

  let file: t = "file";
  let memory: t = "memory";

  let toString = (v: t) => v;

  let isAllowedScheme = scheme => {
    scheme == file || scheme == memory;
  };

  let of_yojson = json =>
    switch (json) {
    | `String(scheme) when isAllowedScheme(scheme) => Ok(scheme)
    | `List([`String(scheme), ..._]) when isAllowedScheme(scheme) =>
      Ok(scheme)
    | _ => Error("Invalid scheme")
    };

  let to_yojson = v => `String(v);
};

[@deriving yojson({strict: false})]
type t = {
  scheme: Scheme.t,
  path: string,
};

let fromMemory = (path: string) => {scheme: Scheme.memory, path};
let fromPath = (path: string) => {scheme: Scheme.file, path};

let toString = (uri: t) => {
  //Scheme.toString(uri.scheme) ++ "://" ++ uri.path;
  "file://" ++ uri.path;
};

let getScheme = (uri: t) => uri.scheme;
