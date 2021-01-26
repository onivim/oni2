open Oni_Core;
open Utility;
open Oniguruma;

module Snippet = Snippet;

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

[@deriving show]
type command =
  | JumpToNextPlaceholder
  | JumpToPreviousPlaceholder
  | InsertSnippet([@opaque] Snippet.t); // TODO: How to have payload

[@deriving show]
type msg =
  | Command(command)
  | SnippetInsertionError(string);

type model = unit;

type outmsg =
  | Effect(Isolinear.Effect.t(msg))
  | Nothing;

let update = (_msg, model) => (model, Nothing);

module Commands = {
  open Feature_Commands.Schema;

  let nextPlaceholder =
    define(
      ~category="Snippets",
      ~title="Jump to next snippet placeholder",
      "jumpToNextSnippetPlaceholder",
      Command(JumpToNextPlaceholder),
    );

  let previousPlaceholder =
    define(
      ~category="Snippets",
      ~title="Jump to previous snippet placeholder",
      "jumpToPrevSnippetPlaceholder",
      Command(JumpToPreviousPlaceholder),
    );

  let snippetCommandParser = json => {
    open Oni_Core.Json.Decode;

    // Decode {"snippets": "some-snippet-text"}
    let snippets =
      obj(({field, _}) => {
        field.required(
          "snippet",
          string
          |> and_then(str => {
               switch (Snippet.parse(str)) {
               | Ok(snippet) => succeed(snippet)
               | Error(msg) => fail(msg)
               }
             }),
        )
      });

    let decode =
      one_of([
        // TODO: Decoder for getting snippet by name
        ("snippets", snippets),
      ]);

    let snippetResult = json |> decode_value(decode);

    switch (snippetResult) {
    | Ok(snippet) => Command(InsertSnippet(snippet))
    | Error(msg) => SnippetInsertionError(string_of_error(msg))
    };
  };

  let insertSnippet =
    defineWithArgs(
      ~category="Snippets",
      ~title="Insert snippet",
      "editor.action.insertSnippet",
      snippetCommandParser,
    );
};

module ContextKeys = {
  open WhenExpr.ContextKeys.Schema;

  // TODO:
  let inSnippetMode = bool("inSnippetMode", (_: model) => false);
};

module Contributions = {
  let commands =
    Commands.[nextPlaceholder, previousPlaceholder, insertSnippet];

  let contextKeys = model => {
    WhenExpr.ContextKeys.(
      ContextKeys.[inSnippetMode] |> Schema.fromList |> fromSchema(model)
    );
  };
};
