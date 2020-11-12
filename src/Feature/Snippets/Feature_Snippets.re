open Oniguruma;

let placeholderRegex = OnigRegExp.create("\\$\\{[0-9]+.*\\}|\\$[0-9]*");

// TODO: This is just a stub for now -
// We need full snippet state management around placeholders, UX, etc.
let snippetToInsert = (~snippet: string) => {
  placeholderRegex
  |> Result.map(regex => {
       let firstMatch = OnigRegExp.Fast.search(snippet, 0, regex);
       if (firstMatch < 0) {
         snippet;
       } else if (firstMatch < String.length(snippet)) {
         String.sub(snippet, 0, firstMatch);
       } else {
         snippet;
       };
     })
  |> Result.value(~default=snippet);
};

let%test "pass-through with no placeholders" = {
  snippetToInsert(~snippet="Hello, world") == "Hello, world";
};

let%test "clips at first placeholder" = {
  snippetToInsert(~snippet="Hello ($1)") == "Hello (";
};

let%test "clips at first placeholder w/ default" = {
  snippetToInsert(~snippet="Hello (${1:expr})") == "Hello (";
};
