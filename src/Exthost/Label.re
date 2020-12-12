open Oni_Core;

[@deriving show]
type segment =
  | Text(string)
  | Icon(string);

[@deriving show]
type t = {
  parsed: list(segment),
  raw: string,
};

let segments = ({parsed, _}) => parsed;

module Parse = {
  open Oniguruma;
  let iconMatcher = OnigRegExp.create("\\$\\((.*)\\)") |> Result.get_ok;

  let rec loop = str => {
    let len = String.length(str);
    let matches = OnigRegExp.search(str, 0, iconMatcher);

    if (Array.length(matches) == 0) {
      [Text(str)];
    } else {
      let matchPos = matches[0].startPos;
      let matchEnd = matches[0].endPos;

      if (matchPos > 0) {
        [
          Text(String.sub(str, 0, matchPos)),
          ...loop(String.sub(str, matchPos, len - matchPos)),
        ];
      } else {
        let text = OnigRegExp.Match.getText(matches[1]);
        if (matchEnd == len) {
          [Icon(text)];
        } else {
          [Icon(text), ...loop(String.sub(str, matchEnd, len - matchEnd))];
        };
      };
    };
  };

  let parse = str => {
    let len = String.length(str);

    if (len == 0) {
      [];
    } else if (len == 1) {
      [Text(str)];
    } else {
      loop(str);
    };
  };
};

module Decode = {
  open Json.Decode;
  let decode: Json.decoder(t) =
    string |> map(str => {parsed: Parse.parse(str), raw: str});
};

module Encode = {
  open Json.Encode;

  let encode = label => label.raw |> string;
};

let decode = Decode.decode;
let encode = Encode.encode;
let ofString = str => {parsed: Parse.parse(str), raw: str};

let toString = ({raw, _}) => raw;
