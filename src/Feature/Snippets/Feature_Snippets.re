open Oni_Core;
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

module Session = {
  type t = {
    editorId: int,
    snippet: Snippet.t,
    startLine: LineNumber.t,
    lineCount: int,
    currentPlaceholder: int,
  };

  let startLine = ({startLine, _}) => startLine;
  let stopLine = ({startLine, lineCount, _}) =>
    EditorCoreTypes.LineNumber.(startLine + lineCount);

  let start = (~editorId, ~position: BytePosition.t, ~snippet) => {
    let placeholders = Snippet.placeholders(snippet);
    let currentPlaceholder = Snippet.Placeholder.initial(placeholders);
    let lineCount = List.length(snippet);
    {
      editorId,
      snippet,
      currentPlaceholder,
      startLine: position.line,
      lineCount,
    };
  };

  let remapPositions = (~startLine, positions) => {
    let startLineIdx = EditorCoreTypes.LineNumber.toZeroBased(startLine);

    let remapRange = (range: ByteRange.t) => {
      ByteRange.{
        start: {
          line: LineNumber.(range.start.line + startLineIdx),
          byte: range.start.byte,
        },
        stop: {
          line: LineNumber.(range.stop.line + startLineIdx),
          byte: range.stop.byte,
        },
      };
    };

    let remapPosition = (position: BytePosition.t) => {
      BytePosition.{
        line: LineNumber.(position.line + startLineIdx),
        byte: position.byte,
      };
    };

    switch (positions) {
    | Snippet.Placeholder.Ranges(ranges) =>
      Snippet.Placeholder.Ranges(ranges |> List.map(remapRange))
    | Snippet.Placeholder.Positions(positions) =>
      Snippet.Placeholder.Positions(positions |> List.map(remapPosition))
    };
  };

  let next = ({snippet, currentPlaceholder, _} as session) => {
    let placeholders = Snippet.placeholders(snippet);

    let newPlaceholder =
      Snippet.Placeholder.next(~placeholder=currentPlaceholder, placeholders);
    {...session, currentPlaceholder: newPlaceholder};
  };

  let previous = ({snippet, currentPlaceholder, _} as session) => {
    let placeholders = Snippet.placeholders(snippet);

    let newPlaceholder =
      Snippet.Placeholder.previous(
        ~placeholder=currentPlaceholder,
        placeholders,
      );
    {...session, currentPlaceholder: newPlaceholder};
  };

  let getPlaceholderPositions = ({snippet, currentPlaceholder, startLine, _}) => {
    let placeholders = Snippet.placeholders(snippet);
    Snippet.Placeholder.positions(
      ~placeholders,
      ~index=currentPlaceholder,
      snippet,
    )
    |> Option.map(remapPositions(~startLine));
  };

  let isComplete = ({snippet, currentPlaceholder, _}) => {
    let placeholders = Snippet.placeholders(snippet);
    Snippet.Placeholder.final(placeholders) == currentPlaceholder;
  };
};

[@deriving show]
type command =
  | JumpToNextPlaceholder
  | JumpToPreviousPlaceholder
  | InsertSnippet([@opaque] Snippet.t);

[@deriving show]
type msg =
  | Command(command)
  | SnippetInserted([@opaque] Session.t)
  | SnippetInsertionError(string);

type model = {maybeSession: option(Session.t)};

let session = ({maybeSession}) => maybeSession;

let isActive = ({maybeSession}) => maybeSession != None;

let modeChanged = (~mode, model) => {
  let isModeValid =
    Vim.Mode.(
      switch (mode) {
      | Select(_)
      | Insert(_) => true
      | _ => false
      }
    );

  if (!isModeValid) {
    {maybeSession: None};
  } else {
    model;
  };
};

let initial = {maybeSession: None};

type outmsg =
  | Effect(Isolinear.Effect.t(msg))
  | ErrorMessage(string)
  | SetCursors(list(BytePosition.t))
  | SetSelections(list(ByteRange.t))
  | Nothing;

module Effects = {
  let startSession =
      (~buffer, ~editorId, ~position: BytePosition.t, ~snippet: Snippet.raw) => {
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

    let session = Session.start(~editorId, ~snippet, ~position);

    let toMsg =
      fun
      | Ok () => SnippetInserted(session)
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

  | SnippetInserted(session) =>
    // Start a session!
    session
    |> Session.getPlaceholderPositions
    |> Option.map(positions => {
         let outmsg =
           switch (positions) {
           | Snippet.Placeholder.Ranges(ranges) => SetSelections(ranges)
           | Snippet.Placeholder.Positions(positions) =>
             SetCursors(positions)
           };

         // Check if we should continue the snippet session
         if (Session.isComplete(session)) {
           ({maybeSession: None}, outmsg);
         } else {
           ({maybeSession: Some(session)}, outmsg);
         };
       })
    |> Option.value(~default=(model, Nothing))

  // TODO
  | Command(JumpToNextPlaceholder) =>
    model.maybeSession
    |> Option.map(session => {
         let session' = Session.next(session);

         let outmsg =
           session'
           |> Session.getPlaceholderPositions
           |> Option.map(positions => {
                switch (positions) {
                | Snippet.Placeholder.Ranges(ranges) => SetSelections(ranges)
                | Snippet.Placeholder.Positions(positions) =>
                  SetCursors(positions)
                }
              })
           |> Option.value(~default=Nothing);

         if (Session.isComplete(session')) {
           ({maybeSession: None}, outmsg);
         } else {
           ({maybeSession: Some(session')}, outmsg);
         };
       })
    |> Option.value(~default=(model, Nothing))

  | Command(JumpToPreviousPlaceholder) =>
    model.maybeSession
    |> Option.map(session => {
         let session' = Session.previous(session);

         let outmsg =
           session'
           |> Session.getPlaceholderPositions
           |> Option.map(positions => {
                switch (positions) {
                | Snippet.Placeholder.Ranges(ranges) => SetSelections(ranges)
                | Snippet.Placeholder.Positions(positions) =>
                  SetCursors(positions)
                }
              })
           |> Option.value(~default=Nothing);

         if (Session.isComplete(session')) {
           ({maybeSession: None}, outmsg);
         } else {
           ({maybeSession: Some(session')}, outmsg);
         };
       })
    |> Option.value(~default=(model, Nothing))

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

  let inSnippetMode = bool("inSnippetMode", isActive);
};

module Contributions = {
  let commands =
    Commands.[nextPlaceholder, previousPlaceholder, insertSnippet];

  let contextKeys = model => {
    WhenExpr.ContextKeys.(
      ContextKeys.[inSnippetMode] |> Schema.fromList |> fromSchema(model)
    );
  };
  let keybindings = {
    Feature_Input.Schema.[
      bind(
        ~key="<TAB>",
        ~command=Commands.nextPlaceholder.id,
        ~condition="editorTextFocus && inSnippetMode" |> WhenExpr.parse,
      ),
      bind(
        ~key="<S-TAB>",
        ~command=Commands.previousPlaceholder.id,
        ~condition="editorTextFocus && inSnippetMode" |> WhenExpr.parse,
      ),
    ];
  };
};
