open Oni_Core;

  [@deriving show]
type segment =
  | Text(string)
  | Icon(string);

  [@deriving show]
type t = list(segment);

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
    } else if (str.[0] == '"' && str.[len - 1] == '"') {
      let text = String.sub(str, 1, len - 2);
      loop(text);
    } else {
      [Text(str)];
    };
  };
};

module Decode = {
  open Json.Decode;
  let decode: Json.decoder(t) = string |> map(Parse.parse);
};

let decode = Decode.decode;
let of_string = Parse.parse;
