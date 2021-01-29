open Oni_Core;
open Utility;
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
  type placeholder = {
    placeholderIndex: int,
    // The current line of the current placeholder, in snippet-space
    snippetLineIndex: int,
    // Snippet line contents - used for synchronization
    lineContents: string,
    numberOfPlaceholdersInLine: int,
  };
  type t = {
    editorId: int,
    snippet: Snippet.t,
    startLine: LineNumber.t,
    lineCount: int,
    currentPlaceholder: option(placeholder),
  };

  let startLine = ({startLine, _}) => startLine;
  let stopLine = ({startLine, lineCount, _}) =>
    EditorCoreTypes.LineNumber.(startLine + lineCount);

  let start = (~editorId, ~position: BytePosition.t, ~snippet) => {
    let lines = Snippet.toLines(snippet);
    let placeholders = Snippet.placeholders(snippet);
    let currentPlaceholder = Snippet.Placeholder.initial(placeholders);
    let maybePlaceholderLine =
      Snippet.getFirstLineIndexWithPlaceholder(
        ~index=currentPlaceholder,
        snippet,
      );
    let maybePlaceholderCount =
      maybePlaceholderLine
      |> Option.map(line => {
           Snippet.getPlaceholderCountForLine(
             ~index=currentPlaceholder,
             ~line,
             snippet,
           )
         });

    let currentPlaceholder =
      OptionEx.map2(
        (placeholderLine, placeholderCount) =>
          {
            placeholderIndex: currentPlaceholder,
            snippetLineIndex: placeholderLine,
            numberOfPlaceholdersInLine: placeholderCount,
            lineContents: lines[placeholderLine],
          },
        maybePlaceholderLine,
        maybePlaceholderCount,
      );

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

  // [synchronize(~buffer, state)] updates our internal snippet state to reflect any updates
  let synchronize =
      (
        ~buffer: Oni_Core.Buffer.t,
        {snippet, currentPlaceholder, startLine, _} as session,
      ) => {
    currentPlaceholder
    |> OptionEx.flatMap(
         ({placeholderIndex, snippetLineIndex, lineContents, _} as cur) => {
         let bufferLineIdx =
           LineNumber.(startLine + snippetLineIndex) |> LineNumber.toZeroBased;

         if (bufferLineIdx < Buffer.getNumberOfLines(buffer)) {
           let newLineContents =
             Buffer.getLine(bufferLineIdx, buffer) |> BufferLine.raw;

           // If the 'before' line is equal to our new line, nothing happened -
           // we're already synchronized
           if (String.equal(lineContents, newLineContents)) {
             Some(session);
           } else {
             // The lines are _not_ equal, so let's figure out the delta,
             // and then update the current placeholder in the snippet to reflect
             // the new state.
             let originalLineLength = String.length(lineContents);
             let newLineLength = String.length(newLineContents);

             // Calculate the delta in characters.
             let delta = newLineLength - originalLineLength;

             // However, we also need to account for the number of placeholders in line
             // - if there are two placeholders, there will be twice the delta.

             let placeholderCount =
               Snippet.getPlaceholderCountForLine(
                 ~index=placeholderIndex,
                 ~line=snippetLineIndex,
                 snippet,
               );
             if (placeholderCount > 0) {
               let normalizedDelta = delta / placeholderCount;
               let placeholders = Snippet.placeholders(snippet);

               // Now... we need to find the original range to get the
               // exact substring text.
               Snippet.Placeholder.positions(
                 ~placeholders,
                 ~index=placeholderIndex,
                 snippet,
               )
               |> OptionEx.flatMap(positions => {
                    let line =
                      EditorCoreTypes.LineNumber.ofZeroBased(
                        snippetLineIndex,
                      );
                    switch (positions) {
                    | Snippet.Placeholder.Positions(_) =>
                      Some(
                        ByteRange.{
                          start: {
                            line,
                            byte: ByteIndex.zero,
                          },
                          stop: {
                            line,
                            byte: ByteIndex.ofInt(-1),
                          },
                        },
                      )
                    | Snippet.Placeholder.Ranges(ranges) =>
                      let rangesForLine =
                        ranges
                        |> List.filter((range: ByteRange.t) =>
                             range.start.line == line
                           )
                        |> List.sort(EditorCoreTypes.ByteRange.compare);

                      List.nth_opt(rangesForLine, 0);
                    };
                  })
               |> OptionEx.flatMap((originalRange: ByteRange.t) => {
                    let startByte = originalRange.start.byte;
                    let stopByte = originalRange.stop.byte;
                    let originalByteLength =
                      ByteIndex.toInt(stopByte)
                      - ByteIndex.toInt(startByte)
                      + 1;
                    let newByteLength = normalizedDelta + originalByteLength;
                    // We erased past the placeholder, so time to close the session..
                    if (newByteLength < 0) {
                      None;
                    } else if (newByteLength == 0) {
                      Some("");
                    } else {
                      let newText =
                        String.sub(
                          newLineContents,
                          ByteIndex.toInt(startByte),
                          newByteLength,
                        );
                      Some(newText);
                    };
                  })
               |> Option.map(newPlaceholderText => {
                    let snippet' =
                      Snippet.updatePlaceholder(
                        ~index=placeholderIndex,
                        ~text=newPlaceholderText,
                        snippet,
                      );

                    {
                      ...session,
                      snippet: snippet',
                      currentPlaceholder:
                        Some({...cur, lineContents: newLineContents}),
                    };
                  });
             } else {
               None;
             };
           };
         } else {
           None;
         };
       });
  };

  let next = (~buffer, session) => {
    synchronize(~buffer, session)
    |> Option.map(session => {
         let {snippet, currentPlaceholder, _} = session;
         let placeholders = Snippet.placeholders(snippet);

         let currentPlaceholder': option(placeholder) =
           currentPlaceholder
           |> OptionEx.flatMap(placeholder => {
                let newPlaceholder =
                  Snippet.Placeholder.next(
                    ~placeholder=placeholder.placeholderIndex,
                    placeholders,
                  );
                Snippet.getFirstLineIndexWithPlaceholder(
                  ~index=newPlaceholder,
                  snippet,
                )
                |> Option.map(line => {
                     let count =
                       Snippet.getPlaceholderCountForLine(
                         ~index=newPlaceholder,
                         ~line,
                         snippet,
                       );
                     let lines = Snippet.toLines(snippet);
                     {
                       placeholderIndex: newPlaceholder,
                       snippetLineIndex: line,
                       lineContents: lines[line],
                       numberOfPlaceholdersInLine: count,
                     };
                   });
              });
         {...session, currentPlaceholder: currentPlaceholder'};
       });
  };

  let previous = (~buffer, session) => {
    synchronize(~buffer, session)
    |> Option.map(session => {
         let {snippet, currentPlaceholder, _} = session;
         let placeholders = Snippet.placeholders(snippet);

         let currentPlaceholder': option(placeholder) =
           currentPlaceholder
           |> OptionEx.flatMap(placeholder => {
                let newPlaceholder =
                  Snippet.Placeholder.previous(
                    ~placeholder=placeholder.placeholderIndex,
                    placeholders,
                  );
                Snippet.getFirstLineIndexWithPlaceholder(
                  ~index=newPlaceholder,
                  snippet,
                )
                |> Option.map(line => {
                     let count =
                       Snippet.getPlaceholderCountForLine(
                         ~index=newPlaceholder,
                         ~line,
                         snippet,
                       );
                     let lines = Snippet.toLines(snippet);
                     {
                       placeholderIndex: newPlaceholder,
                       snippetLineIndex: line,
                       lineContents: lines[line],
                       numberOfPlaceholdersInLine: count,
                     };
                   });
              });
         {...session, currentPlaceholder: currentPlaceholder'};
       });
  };

  let getPlaceholderPositions = ({snippet, currentPlaceholder, startLine, _}) => {
    let placeholders = Snippet.placeholders(snippet);
    currentPlaceholder
    |> Option.map(({placeholderIndex, _}) => placeholderIndex)
    |> OptionEx.flatMap(placeholderIndex => {
         Snippet.Placeholder.positions(
           ~placeholders,
           ~index=placeholderIndex,
           snippet,
         )
       })
    |> Option.map(remapPositions(~startLine));
  };

  let isComplete = ({snippet, currentPlaceholder, _}) => {
    let placeholders = Snippet.placeholders(snippet);
    switch (currentPlaceholder) {
    | None => true
    | Some({placeholderIndex, _}) =>
      placeholderIndex == Snippet.Placeholder.final(placeholders)
    };
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
      (
        ~resolverFactory,
        ~buffer,
        ~editorId,
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
      Snippet.resolve(
        ~getVariable=resolverFactory(),
        ~indentationSettings,
        ~prefix,
        ~postfix,
        snippet,
      );

    let lines = Snippet.toLines(resolvedSnippet);

    if (Array.length(lines) > 0) {
      let session =
        Session.start(~editorId, ~snippet=resolvedSnippet, ~position);

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
    } else {
      Isolinear.Effect.none;
    };
  };
};

let update =
    (~resolverFactory, ~maybeBuffer, ~editorId, ~cursorPosition, msg, model) =>
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
    maybeBuffer
    |> OptionEx.flatMap(buffer => {
         model.maybeSession
         |> OptionEx.flatMap(Session.next(~buffer))
         |> Option.map(session' => {
              let outmsg =
                session'
                |> Session.getPlaceholderPositions
                |> Option.map(positions => {
                     switch (positions) {
                     | Snippet.Placeholder.Ranges(ranges) =>
                       SetSelections(ranges)
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
       })
    |> Option.value(~default=(model, Nothing))

  | Command(JumpToPreviousPlaceholder) =>
    maybeBuffer
    |> OptionEx.flatMap(buffer => {
         model.maybeSession
         |> OptionEx.flatMap(Session.previous(~buffer))
         |> Option.map(session' => {
              let outmsg =
                session'
                |> Session.getPlaceholderPositions
                |> Option.map(positions => {
                     switch (positions) {
                     | Snippet.Placeholder.Ranges(ranges) =>
                       SetSelections(ranges)
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
       })
    |> Option.value(~default=(model, Nothing))

  | Command(InsertSnippet(snippet)) =>
    let eff =
      maybeBuffer
      |> Option.map(buffer => {
           Effects.startSession(
             ~resolverFactory,
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
