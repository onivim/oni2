/*
 * LocalizedToken.re
 */

type t = {
  raw: string,
  token: option(string),
  localized: option(string),
}

open Oni_Core.Utility;

let regex = Re.Posix.re("^%(.*)%$") |> Re.compile;

let parse = raw => {
  let token = raw
  |> Re.exec_opt(regex)
  |> Option.map((g) => Re.Group.get(g, 1));

  { raw, token, localized: None }
};

let localize = (dictionary: LocalizationDictionary.t, {raw, token, _}: t) => {
  let localized = token
  |> Option.bind((token) => LocalizationDictionary.get(token, dictionary));

  { raw, token, localized };
};

exception LocalizedTokenParseException;

let pp = (formatter, v) => "";

let of_yojson = fun
| `String(v) => Ok(parse(v))
| _ => Error("Expected string");

let of_yojson_exn = fun
| `String(v) => parse(v)
| _ => raise(LocalizedTokenParseException);

let to_yojson = v => `Null;
