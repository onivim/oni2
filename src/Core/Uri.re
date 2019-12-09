module Scheme = {
  [@deriving show]
  type t =
    | File
    | Http
    | Https
    | Memory;

  let create = () => {
    let strToScheme = Hashtbl.create(8);
    Hashtbl.add(strToScheme, "file", File);
    Hashtbl.add(strToScheme, "http", Http);
    Hashtbl.add(strToScheme, "https", Https);
    Hashtbl.add(strToScheme, "memory", Memory);
    strToScheme;
  };

  let strToScheme = create();

  let toString =
    fun
    | File => "file"
    | Http => "http"
    | Https => "https"
    | Memory => "memory";

  let _isAllowedScheme = Hashtbl.mem(strToScheme);

  let _ofString = Hashtbl.find(strToScheme);

  let of_yojson = json =>
    switch (json) {
    | `String(scheme) when _isAllowedScheme(scheme) =>
      Ok(scheme |> _ofString)
    | `List([`String(scheme), ..._]) when _isAllowedScheme(scheme) =>
      Ok(scheme |> _ofString)
    | _ => Error("Invalid scheme")
    };

  let to_yojson = v => `String(v |> toString);
};

[@deriving (show, yojson({strict: false}))]
type t = {
  scheme: Scheme.t,
  path: string,
};

let fromMemory = (path: string) => {scheme: Scheme.Memory, path};
let fromPath = (path: string) => {scheme: Scheme.File, path};

let toString = (uri: t) => {
  Scheme.toString(uri.scheme) ++ "://" ++ uri.path;
};

let getScheme = (uri: t) => uri.scheme;
