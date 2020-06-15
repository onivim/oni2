open Oni_Core;

[@deriving show]
type t = {
  language: option(string),
  scheme: option(string),
  exclusive: bool,
};

let decode = {
  Json.Decode.(
    obj(({field, _}) =>
      {
        language: field.optional("language", string),
        scheme: field.optional("scheme", string),
        exclusive: field.withDefault("exclusive", true, bool),
      }
    )
  );
};

let matches = (~filetype: string, filter) =>
  filter.language == Some(filetype);

let toString = filter =>
  Printf.sprintf(
    "DocumentFilter : language = %s, scheme = %s, exclusive = %b",
    filter.language |> Option.value(~default="(none)"),
    filter.scheme |> Option.value(~default="(none)"),
    filter.exclusive,
  );
