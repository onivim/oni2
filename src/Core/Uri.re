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

let _slash = "/";
let _slashChar = '/';

let aCode = Char.code('A');
let zCode = Char.code('Z');

let _isDriveLetter = (candidate: Char.t) => {
  Char.code(candidate) >= aCode && Char.code(candidate) <= zCode;
};

let _normalizePath = (scheme: Scheme.t, path: string) => {
  switch (scheme) {
  | File =>
    let pathLen = String.length(path);
    if (pathLen > 1) {
      let firstLetter = path.[0];
      let secondCharacter = path.[1];
      if (_isDriveLetter(firstLetter) && secondCharacter == ':') {
        let firstLetterString =
          String.make(1, Char.lowercase_ascii(firstLetter));
        firstLetterString ++ ":" ++ String.sub(path, 2, pathLen - 2);
      } else {
        path;
      };
    } else {
      path;
    };
  | _ => path
  };
};

// Port over https://github.com/onivim/vscode-exthost/blob/952a57d2205aba6cabfd277d15787ace3b07f7ba/src/vs/base/common/uri.ts#L75
// VSCode uses a slash-character as the default base - so a windows-path
// like C:/test gets stored as /C:test
let _addSlash = (scheme: Scheme.t, path: string) => {
  switch (scheme) {
  | Https
  | Http
  | File =>
    if (String.length(path) == 0) {
      _slash;
    } else if (path.[0] != _slashChar) {
      _slash ++ path;
    } else {
      path;
    }
  | Memory => path
  };
};

let _referenceResolution = (scheme: Scheme.t, path: string) => {
  path |> _normalizePath(scheme) |> _addSlash(scheme);
};

let fromScheme = (~scheme: Scheme.t, path: string) => {
  scheme,
  path: _referenceResolution(scheme, path),
};

let fromMemory = fromScheme(~scheme=Scheme.Memory);
let fromPath = fromScheme(~scheme=Scheme.File);

let toString = (uri: t) => {
  Scheme.toString(uri.scheme) ++ "://" ++ uri.path;
};

let getScheme = (uri: t) => uri.scheme;
