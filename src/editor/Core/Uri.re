module Scheme = {
[@deriving (show({with_path: false}), yojson({strict: false}))]
type t =
  | [@name "file"] File
  | [@name "memory"] Memory;

let toString = (v: t) =>
  switch (v) {
  | File => "file"
  | Memory => "memory"
  };
};

[@deriving (show({with_path: false}), yojson({strict: false}))]
type t = {
scheme: Scheme.t,
path: string,
};

let fromMemory = (path: string) => {scheme: Scheme.Memory, path};
let fromPath = (path: string) => {scheme: Scheme.File, path};

let show = (uri: t) => {
    Scheme.toString(uri.scheme) ++ "://" ++ uri.path;    
};
