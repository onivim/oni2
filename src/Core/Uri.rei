module Scheme: {
  type t =
    | File
    | Http
    | Https
    | Memory
    | Git
    | Custom(string);

  let toString: t => string;

  let of_yojson: Yojson.Safe.t => result(t, string);
  let to_yojson: t => Yojson.Safe.t;

  let decode: Json.decoder(t);
  let encode: Json.encoder(t);
};

type t;

let toString: t => string;

let to_yojson: t => Yojson.Safe.t;
let of_yojson: Yojson.Safe.t => result(t, string);

let fromMemory: string => t;
let fromPath: string => t;
let fromFilePath: FpExp.t(FpExp.absolute) => t;
let fromScheme:
  (~scheme: Scheme.t, ~authority: string=?, ~query: string=?, string) => t;

let toFileSystemPath: t => string;

let getScheme: t => Scheme.t;

let pp: (Format.formatter, t) => unit;

let decode: Json.decoder(t);
let encode: Json.encoder(t);
