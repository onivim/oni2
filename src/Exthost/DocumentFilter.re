open Oni_Core;

[@deriving show]
type t = {
  language: option(string),
  pattern: option(Glob.t),
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
        pattern: field.optional("pattern", Glob.decode),
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
      Glob.matches(glob, Utility.Path.normalizeBackSlashes(filepath))
    };

  fileTypeMatches && patternMatches;
};

let toString = filter =>
  Printf.sprintf(
    "DocumentFilter : language=%s, pattern=%s, scheme=%s, exclusive=%b",
    filter.language |> Option.value(~default="(none)"),
    filter.pattern
    |> Option.map(Glob.toDebugString)
    |> Option.value(~default="(none)"),
    filter.scheme |> Option.value(~default="(none)"),
    filter.exclusive,
  );
