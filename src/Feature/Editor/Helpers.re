open EditorCoreTypes;
open Oni_Core;
open Oni_Core.Utility;

module BufferHighlights = Oni_Syntax.BufferHighlights;

let getTokensForLine =
    (
      ~editor,
      ~bufferHighlights,
      ~cursorLine,
      ~colors: Colors.t,
      ~matchingPairs,
      ~bufferSyntaxHighlights: Feature_Syntax.t,
      ~ignoreMatchingPairs=false,
      ~selection=None,
      startIndex,
      endIndex,
      i,
    ) =>
  if (i >= Editor.totalViewLines(editor)) {
    [];
  } else {
    let bufferId = Editor.getBufferId(editor);
    let bufferLine = Editor.viewLineToBufferLine(~line=i, editor);
    let bufferLineIdx = bufferLine |> Index.fromZeroBased;
    let highlights =
      BufferHighlights.getHighlightsByLine(
        ~bufferId,
        ~line=bufferLineIdx,
        bufferHighlights,
      );

    let isActiveLine = bufferLine == cursorLine;
    let defaultBackground =
      isActiveLine ? colors.lineHighlightBackground : colors.editorBackground;

    let matchingPairIndex =
      ignoreMatchingPairs
        ? None
        : matchingPairs
          |> OptionEx.flatMap(((startPos: Location.t, endPos: Location.t)) =>
               if (Index.toZeroBased(startPos.line) == i) {
                 Some(Index.toZeroBased(startPos.column));
               } else if (Index.toZeroBased(endPos.line) == i) {
                 Some(Index.toZeroBased(endPos.column));
               } else {
                 None;
               }
             );

    let tokenColors =
      Feature_Syntax.getTokens(
        ~bufferId,
        ~line=bufferLineIdx,
        bufferSyntaxHighlights,
      );

    let colorizer =
      BufferLineColorizer.create(
        ~defaultBackgroundColor=defaultBackground,
        ~defaultForegroundColor=colors.editorForeground,
        ~selectionHighlights=selection,
        ~selectionColor=colors.selectionBackground,
        ~matchingPair=matchingPairIndex,
        ~searchHighlights=highlights,
        ~searchHighlightColor=colors.findMatchBackground,
        tokenColors,
      );

    let {tokens}: Editor.viewTokens =
      Editor.viewTokens(~colorizer, ~line=i, ~position=0, editor);

    tokens;
  };

let getTokenAtPosition =
    (
      ~editor,
      ~bufferHighlights,
      ~cursorLine,
      ~colors,
      ~matchingPairs,
      ~bufferSyntaxHighlights,
      ~startIndex,
      ~endIndex,
      position: Location.t,
    ) => {
  let lineNumber = position.line |> Index.toZeroBased;
  let index = position.column |> Index.toZeroBased;

  getTokensForLine(
    ~editor,
    ~bufferHighlights,
    ~cursorLine,
    ~colors,
    ~matchingPairs,
    ~bufferSyntaxHighlights,
    ~ignoreMatchingPairs=true,
    startIndex,
    endIndex,
    lineNumber,
  )
  |> List.filter((token: BufferViewTokenizer.t) => {
       let tokenStart = token.startPosition |> Index.toZeroBased;
       let tokenEnd = token.endPosition |> Index.toZeroBased;
       index >= tokenStart && index < tokenEnd;
     })
  |> Utility.OptionEx.of_list;
};
