open EditorCoreTypes;
open Oni_Core;

module BufferHighlights = Oni_Syntax.BufferHighlights;

let bufferPositionToPixel = (~context: Draw.context, line, char) => {
  let x = float(char) *. context.charWidth -. context.scrollX;
  let y = float(line) *. context.charHeight -. context.scrollY;
  (x, y);
};

let getTokensForLine =
    (
      ~buffer,
      ~bufferHighlights,
      ~cursorLine,
      ~theme: Theme.t,
      ~matchingPairs,
      ~bufferSyntaxHighlights,
      ~ignoreMatchingPairs=false,
      ~selection=None,
      startIndex,
      endIndex,
      i,
    ) =>
  if (i >= Buffer.getNumberOfLines(buffer)) {
    [];
  } else {
    let line = Buffer.getLine(i, buffer);

    let idx = Index.fromZeroBased(i);
    let highlights =
      BufferHighlights.getHighlightsByLine(
        ~bufferId=Buffer.getId(buffer),
        ~line=idx,
        bufferHighlights,
      );

    let isActiveLine = i == cursorLine;
    let defaultBackground =
      isActiveLine
        ? theme.editorLineHighlightBackground : theme.editorBackground;

    let matchingPairIndex =
      switch (matchingPairs) {
      | None => None
      | Some((startPos: Location.t, endPos: Location.t))
          when !ignoreMatchingPairs =>
        if (Index.toZeroBased(startPos.line) == i) {
          Some(Index.toZeroBased(startPos.column));
        } else if (Index.toZeroBased(endPos.line) == i) {
          Some(Index.toZeroBased(endPos.column));
        } else {
          None;
        }
      | _ => None
      };

    let tokenColors =
      Feature_Syntax.getTokens(
        Buffer.getId(buffer),
        Index.fromZeroBased(i),
        bufferSyntaxHighlights,
      );

    let colorizer =
      BufferLineColorizer.create(
        ~startIndex,
        ~endIndex,
        ~defaultBackgroundColor=defaultBackground,
        ~defaultForegroundColor=theme.editorForeground,
        ~selectionHighlights=selection,
        ~selectionColor=theme.editorSelectionBackground,
        ~matchingPair=matchingPairIndex,
        ~searchHighlights=highlights,
        ~searchHighlightColor=theme.editorFindMatchBackground,
        tokenColors,
      );

    BufferViewTokenizer.tokenize(~startIndex, ~endIndex, line, colorizer);
  };

let getTokenAtPosition =
    (
      ~buffer,
      ~bufferHighlights,
      ~cursorLine,
      ~theme,
      ~matchingPairs,
      ~bufferSyntaxHighlights,
      ~startIndex,
      ~endIndex,
      position: Location.t,
    ) => {
  let lineNumber = position.line |> Index.toZeroBased;
  let index = position.column |> Index.toZeroBased;

  getTokensForLine(
    ~buffer,
    ~bufferHighlights,
    ~cursorLine,
    ~theme,
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
