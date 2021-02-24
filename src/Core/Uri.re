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

  let ofString = str =>
    Internal.isKnownScheme(str) ? str |> Internal.ofString : Custom(str);

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
    | `List([`String(scheme), ..._]) => Ok(ofString(scheme))
    | _ => Error("Invalid scheme")
    };

  let decode = {
    open Json.Decode;

    let decodeList =
      list(string)
      |> and_then(
           fun
           | [scheme, ..._] => scheme |> ofString |> succeed
           | [] => fail("No scheme"),
         );

    let decodeString = string |> map(ofString);

    one_of([("string", decodeString), ("list", decodeList)]);
  };

  let to_yojson = v => `String(v |> toString);
  let encode = scheme => Json.Encode.(scheme |> toString |> string);
};

[@deriving (show, yojson({strict: false}))]
type t = {
  scheme: Scheme.t,
  path: string,
  query: [@default None] option(string),
  authority: [@default None] option(string),
};

module Internal = {
  let isDriveLetter = (candidate: Char.t) => {
    let aCode = Char.code('A');
    let zCode = Char.code('Z');
    let upperCandidate = Char.uppercase_ascii(candidate);

    Char.code(upperCandidate) >= aCode && Char.code(upperCandidate) <= zCode;
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

let fromScheme = (~scheme: Scheme.t, ~authority=?, ~query=?, path: string) => {
  scheme,
  path: Internal.referenceResolution(scheme, path),
  query,
  authority,
};

let fromMemory = path => fromScheme(~scheme=Scheme.Memory, path);
let fromPath = path => {
  // On Windows - we need to normalize backward slashes to forward slashes,
  // to be in sync with the logic from vscode:
  // https://github.com/onivim/vscode-exthost/blob/c7df89c1cf0087ca5decaf8f6d4c0fd0257a8b7a/src/vs/base/common/uri.ts#L306
  // Also see: https://github.com/onivim/oni2/issues/2282
  let path = Sys.win32 ? Utility.Path.normalizeBackSlashes(path) : path;

  fromScheme(~scheme=Scheme.File, path);
};

let fromFilePath = fp => {
  fp |> FpExp.toString |> fromPath;
};

let toString = ({scheme, authority, path, query}: t) => {
  let schemeStr = Scheme.toString(scheme);
  switch (scheme) {
  | Http
  | Https =>
    let authorityStr = Option.value(~default="", authority);
    let queryStr =
      query |> Option.map(q => "?" ++ q) |> Option.value(~default="");
    Printf.sprintf("%s://%s%s%s", schemeStr, authorityStr, path, queryStr);
  | _ =>
    Printf.sprintf("%s://%s", schemeStr, Internal.addSlash(scheme, path))
  };
};

let toFileSystemPath = (uri: t) => {
  switch (uri.scheme) {
  | File =>
    let pathLen = String.length(uri.path);
    if (pathLen > 2 && uri.path.[0] == '/') {
      let firstLetter = uri.path.[1];
      let secondCharacter = uri.path.[2];

      // Remove the leading slash when using a Windows file system
      if (Internal.isDriveLetter(firstLetter) && secondCharacter == ':') {
        String.sub(uri.path, 1, pathLen - 1);
      } else {
        uri.path;
      };
    } else {
      uri.path;
    };
  | _ => uri.path
  };
};

let getScheme = (uri: t) => uri.scheme;

let encode = uri =>
  Json.Encode.(
    obj([
      ("$mid", Json.Encode.int(1)), // Magic marshaling id for Uri
      ("scheme", uri.scheme |> Scheme.encode),
      ("path", uri.path |> string),
      ("query", uri.query |> nullable(string)),
      ("authority", uri.authority |> nullable(string)),
    ])
  );

let decode = {
  Json.Decode.(
    obj(({field, _}) =>
      {
        scheme: field.required("scheme", Scheme.decode),
        path: field.required("path", string),
        query: field.optional("query", string),
        authority: field.optional("authority", string),
      }
    )
  );
};
