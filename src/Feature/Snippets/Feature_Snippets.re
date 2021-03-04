open Oni_Core;
open Utility;
open EditorCoreTypes;

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
    snippet: ResolvedSnippet.t,
    startLine: LineNumber.t,
    lineCount: int,
    currentPlaceholder: option(placeholder),
  };

  let startLine = ({startLine, _}) => startLine;
  let stopLine = ({startLine, lineCount, _}) =>
    EditorCoreTypes.LineNumber.(startLine + lineCount);

  let editorId = ({editorId, _}) => editorId;

  let start = (~editorId, ~position: BytePosition.t, ~snippet) => {
    let lines = ResolvedSnippet.toLines(snippet);
    let placeholders = ResolvedSnippet.placeholders(snippet);
    let currentPlaceholder =
      ResolvedSnippet.Placeholder.initial(placeholders);
    let maybePlaceholderLine =
      ResolvedSnippet.getFirstLineIndexWithPlaceholder(
        ~index=currentPlaceholder,
        snippet,
      );
    let maybePlaceholderCount =
      maybePlaceholderLine
      |> Option.map(line => {
           ResolvedSnippet.getPlaceholderCountForLine(
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

    let lineCount = ResolvedSnippet.lineCount(snippet);
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
    | ResolvedSnippet.Placeholder.Ranges(ranges) =>
      ResolvedSnippet.Placeholder.Ranges(ranges |> List.map(remapRange))
    | ResolvedSnippet.Placeholder.Positions(positions) =>
      ResolvedSnippet.Placeholder.Positions(
        positions |> List.map(remapPosition),
      )
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
               ResolvedSnippet.getPlaceholderCountForLine(
                 ~index=placeholderIndex,
                 ~line=snippetLineIndex,
                 snippet,
               );
             if (placeholderCount > 0) {
               let normalizedDelta = delta / placeholderCount;
               let placeholders = ResolvedSnippet.placeholders(snippet);

               // Now... we need to find the original range to get the
               // exact substring text.
               ResolvedSnippet.Placeholder.positions(
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
                    | ResolvedSnippet.Placeholder.Positions(_) =>
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
                    | ResolvedSnippet.Placeholder.Ranges(ranges) =>
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
                      ResolvedSnippet.updatePlaceholder(
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
         let placeholders = ResolvedSnippet.placeholders(snippet);

         let currentPlaceholder': option(placeholder) =
           currentPlaceholder
           |> OptionEx.flatMap(placeholder => {
                let newPlaceholder =
                  ResolvedSnippet.Placeholder.next(
                    ~placeholder=placeholder.placeholderIndex,
                    placeholders,
                  );
                ResolvedSnippet.getFirstLineIndexWithPlaceholder(
                  ~index=newPlaceholder,
                  snippet,
                )
                |> Option.map(line => {
                     let count =
                       ResolvedSnippet.getPlaceholderCountForLine(
                         ~index=newPlaceholder,
                         ~line,
                         snippet,
                       );
                     let lines = ResolvedSnippet.toLines(snippet);
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
         let placeholders = ResolvedSnippet.placeholders(snippet);

         let currentPlaceholder': option(placeholder) =
           currentPlaceholder
           |> OptionEx.flatMap(placeholder => {
                let newPlaceholder =
                  ResolvedSnippet.Placeholder.previous(
                    ~placeholder=placeholder.placeholderIndex,
                    placeholders,
                  );
                ResolvedSnippet.getFirstLineIndexWithPlaceholder(
                  ~index=newPlaceholder,
                  snippet,
                )
                |> Option.map(line => {
                     let count =
                       ResolvedSnippet.getPlaceholderCountForLine(
                         ~index=newPlaceholder,
                         ~line,
                         snippet,
                       );
                     let lines = ResolvedSnippet.toLines(snippet);
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
    let placeholders = ResolvedSnippet.placeholders(snippet);
    currentPlaceholder
    |> Option.map(({placeholderIndex, _}) => placeholderIndex)
    |> OptionEx.flatMap(placeholderIndex => {
         ResolvedSnippet.Placeholder.positions(
           ~placeholders,
           ~index=placeholderIndex,
           snippet,
         )
       })
    |> Option.map(remapPositions(~startLine));
  };

  let isComplete = ({snippet, currentPlaceholder, _}) => {
    let placeholders = ResolvedSnippet.placeholders(snippet);
    switch (currentPlaceholder) {
    | None => true
    | Some({placeholderIndex, _}) =>
      placeholderIndex == ResolvedSnippet.Placeholder.final(placeholders)
    };
  };
};

[@deriving show]
type command =
  | JumpToNextPlaceholder
  | JumpToPreviousPlaceholder
  | EditUserSnippets
  | InsertSnippet({
      // If no snippet is provided - we should open the snippet menu
      maybeSnippet: [@opaque] option(Snippet.t),
      maybeReplaceRange: option(ByteRange.t),
    });

[@deriving show]
type msg =
  | Command(command)
  | SnippetInserted([@opaque] Session.t)
  | SnippetInsertionError(string)
  | SnippetsLoadedForPicker(list(Service_Snippets.SnippetWithMetadata.t))
  | InsertInternal({snippetString: string})
  | SnippetFilesLoadedForPicker(list(Service_Snippets.SnippetFileMetadata.t))
  | EditSnippetFileRequested({
      snippetFile: Service_Snippets.SnippetFileMetadata.t,
    })
  | SnippetFileCreatedSuccessfully([@opaque] FpExp.t(FpExp.absolute))
  | SnippetFileCreationError(string);

module Msg = {
  let insert = (~snippet) => InsertInternal({snippetString: snippet});

  let editSnippetFile = (~snippetFile: Service_Snippets.SnippetFileMetadata.t) =>
    EditSnippetFileRequested({snippetFile: snippetFile});
};

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
  | ShowMenu(Feature_Quickmenu.Schema.menu(msg))
  | OpenFile(FpExp.t(FpExp.absolute))
  | Nothing;

module Effects = {
  let startSession =
      (
        ~maybeReplaceRange: option(ByteRange.t),
        ~resolverFactory,
        ~buffer,
        ~editorId,
        ~position: BytePosition.t,
        ~snippet: Snippet.t,
      ) => {
    let bufferId = Oni_Core.Buffer.getId(buffer);
    let indentationSettings = Oni_Core.Buffer.getIndentation(buffer);

    let line =
      buffer
      |> Oni_Core.Buffer.getLine(position.line |> LineNumber.toZeroBased)
      |> Oni_Core.BufferLine.raw;

    let maybeReplaceRange =
      maybeReplaceRange |> Option.map(range => range |> ByteRange.normalize);

    let position =
      switch (maybeReplaceRange) {
      | None => position
      | Some(range) => range.stop
      };

    let (prefix, postfix) =
      Utility.StringEx.splitAt(~byte=ByteIndex.toInt(position.byte), line);

    // Handle the 'meet column' - if we a meet column was provided,
    // as in the case of completion, we may need to remove some characters
    // from the prefix.
    let (prefix, replaceStartLine) =
      switch (maybeReplaceRange) {
      | None => (prefix, position.line)
      | Some(range) =>
        let replaceFromPosition = range.start;
        let replaceStartLine =
          buffer
          |> Oni_Core.Buffer.getLine(
               replaceFromPosition.line |> LineNumber.toZeroBased,
             )
          |> Oni_Core.BufferLine.raw;
        // First, see how many characters we're working with...

        let byteIdx = replaceFromPosition.byte |> ByteIndex.toInt;
        let prefix =
          if (byteIdx == 0) {
            "";
          } else if (byteIdx >= String.length(replaceStartLine)) {
            prefix;
          } else {
            String.sub(replaceStartLine, 0, byteIdx);
          };
        (prefix, replaceFromPosition.line);
      };

    let resolvedSnippet =
      ResolvedSnippet.resolve(
        ~getVariable=resolverFactory(),
        ~indentationSettings,
        ~prefix,
        ~postfix,
        snippet,
      );

    let lines = ResolvedSnippet.toLines(resolvedSnippet);

    let sessionPosition =
      BytePosition.{
        line: replaceStartLine,
        byte: String.length(prefix) |> ByteIndex.ofInt,
      };

    if (Array.length(lines) > 0) {
      let session =
        Session.start(
          ~editorId,
          ~snippet=resolvedSnippet,
          ~position=sessionPosition,
        );

      let toMsg =
        fun
        | Ok () => SnippetInserted(session)
        | Error(msg) => SnippetInsertionError(msg);

      Service_Vim.Effects.setLines(
        ~bufferId,
        ~start=replaceStartLine,
        ~stop=LineNumber.(position.line + 1),
        ~lines,
        toMsg,
      );
    } else {
      Isolinear.Effect.none;
    };
  };

  let insertSnippet = (~replaceRange: option(ByteRange.t), ~snippet: string) => {
    Isolinear.Effect.createWithDispatch(
      ~name="Feature_Snippets.insertSnippet", dispatch => {
      switch (Snippet.parse(snippet)) {
      | Ok(resolvedSnippet) =>
        dispatch(
          Command(
            InsertSnippet({
              maybeSnippet: Some(resolvedSnippet),
              maybeReplaceRange: replaceRange,
            }),
          ),
        )
      | Error(msg) => dispatch(SnippetInsertionError(msg))
      }
    });
  };
};

module Internal = {
  // Helper function to calculate a start position given a range of selections.
  let getReplaceRangeFromSelections = (~buffer, selections) => {
    List.nth_opt(selections, 0)
    |> OptionEx.flatMap((selection: VisualRange.t) => {
         let normalizedRange = selection.range |> ByteRange.normalize;

         let stopLineIdx =
           normalizedRange.stop.line |> EditorCoreTypes.LineNumber.toZeroBased;
         let stopLineBytes =
           Buffer.getLine(stopLineIdx, buffer) |> BufferLine.raw;
         switch (selection.mode) {
         | Vim.Types.Line =>
           Some(
             ByteRange.{
               start:
                 BytePosition.{
                   line: normalizedRange.start.line,
                   byte: ByteIndex.zero,
                 },
               stop:
                 BytePosition.{
                   line: normalizedRange.stop.line,
                   byte: String.length(stopLineBytes) |> ByteIndex.ofInt,
                 },
             },
           )

         | Vim.Types.Character =>
           // The Vim selection range is inclusive, but the snippets expect
           // an exclusive range - so we need to bump the 'stop' character
           // out a byte.

           Some(
             {
               ByteRange.{
                 start: normalizedRange.start,
                 stop:
                   BytePosition.{
                     line: normalizedRange.stop.line,
                     byte:
                       ByteIndex.next(
                         stopLineBytes,
                         normalizedRange.stop.byte,
                       ),
                   },
               };
             },
           )

         // No-op for now
         | Vim.Types.Block
         | Vim.Types.None => None
         };
       });
  };
};

let update =
    (
      ~languageInfo,
      ~resolverFactory,
      ~selections,
      ~maybeBuffer,
      ~editorId,
      ~cursorPosition,
      ~extensions,
      msg,
      model,
    ) =>
  switch (msg) {
  | SnippetInsertionError(msg) => (model, ErrorMessage(msg))

  | SnippetInserted(session) =>
    // Start a session!
    session
    |> Session.getPlaceholderPositions
    |> Option.map(positions => {
         let outmsg =
           switch (positions) {
           | ResolvedSnippet.Placeholder.Ranges(ranges) =>
             SetSelections(ranges)
           | ResolvedSnippet.Placeholder.Positions(positions) =>
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
                     | ResolvedSnippet.Placeholder.Ranges(ranges) =>
                       SetSelections(ranges)
                     | ResolvedSnippet.Placeholder.Positions(positions) =>
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
                     | ResolvedSnippet.Placeholder.Ranges(ranges) =>
                       SetSelections(ranges)
                     | ResolvedSnippet.Placeholder.Positions(positions) =>
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

  | Command(InsertSnippet({maybeSnippet, maybeReplaceRange})) =>
    let eff =
      maybeBuffer
      |> Option.map(buffer => {
           switch (maybeSnippet) {
           | Some(snippet) =>
             Effect(
               Effects.startSession(
                 ~maybeReplaceRange=
                   maybeReplaceRange
                   |> OptionEx.or_lazy(() =>
                        Internal.getReplaceRangeFromSelections(
                          ~buffer,
                          selections,
                        )
                      ),
                 ~resolverFactory,
                 ~buffer,
                 ~editorId,
                 ~position=cursorPosition,
                 ~snippet,
               ),
             )
           | None =>
             let fileType =
               buffer |> Buffer.getFileType |> Buffer.FileType.toString;

             let filePaths =
               Feature_Extensions.snippetFilePaths(~fileType, extensions);
             Effect(
               Service_Snippets.Effect.snippetFromFiles(
                 ~filePaths, ~fileType, snippets =>
                 SnippetsLoadedForPicker(snippets)
               ),
             );
           }
         })
      |> Option.value(~default=Nothing);
    (model, eff);

  | Command(EditUserSnippets) => (
      model,
      Effect(
        Service_Snippets.Effect.getUserSnippetFiles(
          ~languageInfo, snippetFiles =>
          SnippetFilesLoadedForPicker(snippetFiles)
        ),
      ),
    )

  | EditSnippetFileRequested({snippetFile}) =>
    let eff =
      Service_Snippets.Effect.createSnippetFile(
        ~filePath=snippetFile.filePath,
        fun
        | Ok(filePath) => SnippetFileCreatedSuccessfully(filePath)
        | Error(msg) => SnippetFileCreationError(msg),
      );
    (model, Effect(eff));

  | SnippetFileCreatedSuccessfully(filePath) => (model, OpenFile(filePath))

  | SnippetFileCreationError(msg) => (model, ErrorMessage(msg))

  | SnippetsLoadedForPicker(snippetsWithMetadata) =>
    let menu =
      Feature_Quickmenu.Schema.menu(
        ~onItemSelected=
          (item: Service_Snippets.SnippetWithMetadata.t) =>
            InsertInternal({snippetString: item.snippet}),
        ~toString=
          (snippet: Service_Snippets.SnippetWithMetadata.t) => {
            snippet.prefix ++ ":" ++ snippet.description
          },
        snippetsWithMetadata,
      );
    (model, ShowMenu(menu));

  | SnippetFilesLoadedForPicker(snippetFiles) =>
    let filesAndPaths =
      snippetFiles
      |> List.filter_map(
           (snippetFile: Service_Snippets.SnippetFileMetadata.t) => {
           FpExp.baseName(snippetFile.filePath)
           |> Option.map(filePath => {
                let language =
                  snippetFile.language |> Option.value(~default="global");
                let name =
                  snippetFile.isCreated
                    ? Printf.sprintf("Edit %s", filePath)
                    : Printf.sprintf("Create %s", filePath);
                let title = Printf.sprintf("%s: %s", language, name);
                (title, snippetFile);
              })
         });

    let menu =
      Feature_Quickmenu.Schema.menu(
        ~onItemSelected=
          item => EditSnippetFileRequested({snippetFile: item |> snd}),
        ~toString=item => fst(item),
        filesAndPaths,
      );

    (model, ShowMenu(menu));

  | InsertInternal({snippetString}) =>
    let eff =
      maybeBuffer
      |> Option.map(buffer => {
           switch (Snippet.parse(snippetString)) {
           | Ok(snippet) =>
             Effect(
               Effects.startSession(
                 ~maybeReplaceRange=
                   Internal.getReplaceRangeFromSelections(
                     ~buffer,
                     selections,
                   ),
                 ~resolverFactory,
                 ~buffer,
                 ~editorId,
                 ~position=cursorPosition,
                 ~snippet,
               ),
             )
           | Error(msg) => ErrorMessage(msg)
           }
         })
      |> Option.value(~default=Nothing);

    (model, eff);
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
      nullable(
        one_of([
          // TODO: Decoder for getting snippet by name
          ("snippets", snippets),
        ]),
      );

    let snippetResult = json |> decode_value(decode);

    switch (snippetResult) {
    | Ok(snippet) =>
      Command(
        InsertSnippet({maybeSnippet: snippet, maybeReplaceRange: None}),
      )
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

  let editUserSnippets =
    define(
      ~category="Snippets",
      ~title="Configure user snippets",
      "workbench.action.openSnippets",
      Command(EditUserSnippets),
    );
};

module ContextKeys = {
  open WhenExpr.ContextKeys.Schema;

  let inSnippetMode = bool("inSnippetMode", isActive);
};

module Contributions = {
  let commands =
    Commands.[
      nextPlaceholder,
      previousPlaceholder,
      insertSnippet,
      editUserSnippets,
    ];

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
        ~condition=
          "editorTextFocus && inSnippetMode && !suggestWidgetVisible"
          |> WhenExpr.parse,
      ),
      bind(
        ~key="<S-TAB>",
        ~command=Commands.previousPlaceholder.id,
        ~condition=
          "editorTextFocus && inSnippetMode && !suggestWidgetVisible"
          |> WhenExpr.parse,
      ),
    ];
  };
};
