/*
 * LocalizedToken.re
 */

module Json = Oni_Core.Json;

[@deriving show]
type t = {
  raw: string,
  token: option(string),
  localized: option(string),
};

let regex = Re.Posix.re("^%(.*)%$") |> Re.compile;

let parse = raw => {
  let token =
    raw |> Re.exec_opt(regex) |> Option.map(g => Re.Group.get(g, 1));

  {raw, token, localized: None};
};

let localize = (dictionary: LocalizationDictionary.t, {raw, token, _}: t) => {
  let localized =
    Option.bind(token, token => {
      LocalizationDictionary.get(token, dictionary)
    });

  {raw, token, localized};
};

let decode = Json.Decode.(string |> map(parse));

let to_string = ({raw, localized, _}: t) => {
  Option.value(~default=raw, localized);
};
