open EditorCoreTypes;
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
  | SnippetInserted
  | SnippetInsertionError(string);

type model = unit;

let initial = ();

type outmsg =
  | Effect(Isolinear.Effect.t(msg))
  | ErrorMessage(string)
  | Nothing;

module Effects = {
  let startSession =
      (
        ~buffer,
        ~editorId as _,
        ~position: BytePosition.t,
        ~snippet: Snippet.raw,
      ) => {
    let bufferId = Oni_Core.Buffer.getId(buffer);
    let indentationSettings = Oni_Core.Buffer.getIndentation(buffer);

    let line =
      buffer
      |> Oni_Core.Buffer.getLine(position.line |> LineNumber.toZeroBased)
      |> Oni_Core.BufferLine.raw;

    let (prefix, postfix) =
      Utility.StringEx.splitAt(~byte=ByteIndex.toInt(position.byte), line);

    let resolvedSnippet =
      Snippet.resolve(~indentationSettings, ~prefix, ~postfix, snippet);

    let lines = Snippet.toLines(resolvedSnippet);

    let toMsg =
      fun
      | Ok () => SnippetInserted
      | Error(msg) => SnippetInsertionError(msg);

    Service_Vim.Effects.setLines(
      ~bufferId,
      ~start=position.line,
      ~stop=LineNumber.(position.line + 1),
      ~lines,
      toMsg,
    );
  };
};

let update = (~maybeBuffer, ~editorId, ~cursorPosition, msg, model) =>
  switch (msg) {
  | SnippetInsertionError(msg) => (model, ErrorMessage(msg))

  // TODO: Start session!
  | SnippetInserted => (model, Nothing)

  // TODO
  | Command(JumpToNextPlaceholder) => (model, Nothing)

  // TODO
  | Command(JumpToPreviousPlaceholder) => (model, Nothing)

  | Command(InsertSnippet(snippet)) =>
    let eff =
      maybeBuffer
      |> Option.map(buffer => {
           Effects.startSession(
             ~buffer,
             ~editorId,
             ~position=cursorPosition,
             ~snippet,
           )
         })
      |> Option.value(~default=Isolinear.Effect.none);

    (model, Effect(eff));
  };

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
