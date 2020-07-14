open Oni_Core;

[@deriving show]
type t =
  | String(string)
  | ThemeReference(ThemeColor.t);

module Internal = {
  // TODO: Move to a shared utility location, like JsonEx?
  let nestedJsonString: Json.decoder('a) => Json.decoder('a) =
    decoder =>
      Json.Decode.(
        string
        |> and_then((str: string) => {
             let result: result('a, string) =
               str
               |> Yojson.Safe.from_string
               |> decode_value(decoder)
               |> Result.map_error(Json.Decode.string_of_error);

             switch (result) {
             | Ok(parsed) => succeed(parsed)
             | Error(msg) => fail(msg)
             };
           })
      );
};

let decode =
  Json.Decode.(
    one_of([
      (
        "nestedString",
        Internal.nestedJsonString(ThemeColor.decode)
        |> map(themeColor => ThemeReference(themeColor)),
      ),
      ("string", string |> map(str => String(str))),
      (
        "themeColor",
        ThemeColor.decode |> map(themeColor => ThemeReference(themeColor)),
      ),
    ])
  );

let resolve = (colors, color) => {
  switch (color) {
  | String(str) =>
    try(Some(Revery.Color.hex(str))) {
    | _exn => None
    }
  | ThemeReference({id}) =>
    let key = ColorTheme.key(id);
    ColorTheme.Colors.get(key, colors);
  };
};
