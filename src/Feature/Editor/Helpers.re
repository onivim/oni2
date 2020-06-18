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
      ~colors: Colors.t,
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
    let length = BufferLine.lengthInBytes(line);
    let startIndex = max(0, startIndex);
    let endIndex = max(0, endIndex);
    if (length == 0) {
      [];
    } else {
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
          ? colors.lineHighlightBackground : colors.editorBackground;

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
          ~bufferId=Buffer.getId(buffer),
          ~line=Index.fromZeroBased(i),
          bufferSyntaxHighlights,
        );

      let startByte = BufferLine.getByteFromIndex(~index=startIndex, line);

      let colorizer =
        BufferLineColorizer.create(
          ~startByte,
          ~defaultBackgroundColor=defaultBackground,
          ~defaultForegroundColor=colors.editorForeground,
          ~selectionHighlights=selection,
          ~selectionColor=colors.selectionBackground,
          ~matchingPair=matchingPairIndex,
          ~searchHighlights=highlights,
          ~searchHighlightColor=colors.findMatchBackground,
          tokenColors,
        );

      BufferViewTokenizer.tokenize(~startIndex, ~endIndex, line, colorizer);
    };
  };

let getTokenAtPosition =
    (
      ~buffer,
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
    ~buffer,
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
