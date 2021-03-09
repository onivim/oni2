open Oni_Core;

[@deriving show]
type t = {
  language: option(string),
  pattern: option(Re.re),
  scheme: option(string),
  exclusive: bool,
};

module CustomDecoders = {
  let glob =
    Json.Decode.(
      string
      |> and_then(str =>
           try(
             str
             |> Utility.Path.normalizeBackSlashes
             |> Re.Glob.glob(~expand_braces=true)
             |> Re.compile
             |> succeed
           ) {
           | exn => fail(Printexc.to_string(exn))
           }
         )
    );
};

let decode = {
  Json.Decode.(
    obj(({field, _}) =>
      {
        language: field.optional("language", string),
        scheme: field.optional("scheme", string),
        exclusive: field.withDefault("exclusive", true, bool),
        pattern: field.optional("pattern", CustomDecoders.glob),
      }
    )
  );
};

let matches = (~filetype: string, ~filepath: string, filter) => {
  let fileTypeMatches =
    switch (filter.language) {
    | None => true
    | Some(language) => language == filetype
    };

  let patternMatches =
    switch (filter.pattern) {
    | None => true
    | Some(glob) =>
      Re.matches(glob, Utility.Path.normalizeBackSlashes(filepath)) != []
    };

  fileTypeMatches && patternMatches;
};

let toString = filter =>
  Printf.sprintf(
    "DocumentFilter : language = %s, scheme = %s, exclusive = %b",
    filter.language |> Option.value(~default="(none)"),
    filter.scheme |> Option.value(~default="(none)"),
    filter.exclusive,
  );
