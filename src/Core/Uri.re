module Scheme = {
  [@deriving show]
  type t =
    | File
    | Http
    | Https
    | Memory
    | Git
    | Custom(string);

  module Internal = {
    let schemeLookup = {
      let lookup = Hashtbl.create(8);
      Hashtbl.add(lookup, "file", File);
      Hashtbl.add(lookup, "http", Http);
      Hashtbl.add(lookup, "https", Https);
      Hashtbl.add(lookup, "memory", Memory);
      Hashtbl.add(lookup, "git", Git);
      lookup;
    };

    let isKnownScheme = Hashtbl.mem(schemeLookup);
    let ofString = Hashtbl.find(schemeLookup);
  };

  let toString =
    fun
    | File => "file"
    | Http => "http"
    | Https => "https"
    | Memory => "memory"
    | Git => "git"
    | Custom(scheme) => scheme;

  let of_yojson = json =>
    switch (json) {
    | `String(scheme)
    | `List([`String(scheme), ..._]) =>
      Internal.isKnownScheme(scheme)
        ? Ok(scheme |> Internal.ofString) : Ok(Custom(scheme))

    | _ => Error("Invalid scheme")
    };

  let to_yojson = v => `String(v |> toString);
};

[@deriving (show, yojson({strict: false}))]
type t = {
  scheme: Scheme.t,
  path: string,
  query: [@default None] option(string),
};

module Internal = {
  let isDriveLetter = (candidate: Char.t) => {
    let aCode = Char.code('A');
    let zCode = Char.code('Z');

    Char.code(candidate) >= aCode && Char.code(candidate) <= zCode;
  };

  let normalizePath = (scheme: Scheme.t, path: string) => {
    switch (scheme) {
    | File =>
      let pathLen = String.length(path);
      if (pathLen > 1) {
        let firstLetter = path.[0];
        let secondCharacter = path.[1];
        if (isDriveLetter(firstLetter) && secondCharacter == ':') {
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
  let addSlash = (scheme: Scheme.t, path: string) => {
    switch (scheme) {
    | Https
    | Http
    | File =>
      if (String.length(path) == 0) {
        "/";
      } else if (path.[0] != '/') {
        "/" ++ path;
      } else {
        path;
      }
    | Memory
    | Git
    | Custom(_) => path
    };
  };

  let referenceResolution = (scheme: Scheme.t, path: string) =>
    path |> normalizePath(scheme) |> addSlash(scheme);
};

let fromScheme = (~scheme: Scheme.t, ~query=?, path: string) => {
  scheme,
  path: Internal.referenceResolution(scheme, path),
  query,
};

let fromMemory = path => fromScheme(~scheme=Scheme.Memory, path);
let fromPath = path => fromScheme(~scheme=Scheme.File, path);

let toString = (uri: t) => {
  Scheme.toString(uri.scheme) ++ "://" ++ uri.path;
};

let toFileSystemPath = (uri: t) => {
  uri.path;
};

let getScheme = (uri: t) => uri.scheme;
