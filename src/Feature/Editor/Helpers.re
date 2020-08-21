open EditorCoreTypes;
open Oni_Core;

module BufferHighlights = Oni_Syntax.BufferHighlights;

let getTokensForLine =
    (
      ~editor,
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
  if (i >= Editor.totalViewLines(editor)) {
    [];
  } else {
    let viewLine = Editor.viewLine(editor, i);
    let line = viewLine.contents;
    let length = BufferLine.lengthInBytes(line);
    let startIndex = max(0, startIndex);
    let endIndex = max(0, endIndex);
    let bufferId = Editor.getBufferId(editor);

    if (length == 0) {
      [];
    } else {
      let idx = Index.fromZeroBased(i);
      let searchHighlights =
        BufferHighlights.getHighlightsByLine(
          ~bufferId,
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
        | Some((startPos: CharacterPosition.t, endPos: CharacterPosition.t))
            when !ignoreMatchingPairs =>
// TODO before merge:
// Convert character position -> byte position
            None;
//          if (Index.toZeroBased(startPos.line) == i) {
//            Some(Index.toZeroBased(startPos.column));
//          } else if (Index.toZeroBased(endPos.line) == i) {
//            Some(Index.toZeroBased(endPos.column));
//          } else {
//            None;
//          }
        | _ => None
        };

      let tokenColors =
        Feature_Syntax.getTokens(
          ~bufferId,
          ~line=EditorCoreTypes.LineNumber.ofZeroBased(i),
          bufferSyntaxHighlights,
        );

      let startByte = BufferLine.getByteFromIndex(~index=
      startIndex |> CharacterIndex.ofInt, line);

      let colorizer =
        BufferLineColorizer.create(
          ~startByte,
          ~defaultBackgroundColor=defaultBackground,
          ~defaultForegroundColor=colors.editorForeground,
          ~selectionHighlights=selection,
          ~selectionColor=colors.selectionBackground,
          ~matchingPair=matchingPairIndex,
          ~searchHighlights,
          ~searchHighlightColor=colors.findMatchBackground,
          tokenColors,
        );

      BufferViewTokenizer.tokenize(~start=CharacterIndex.ofInt(startIndex),
      ~stop=CharacterIndex.ofInt(endIndex), line, colorizer);
    };
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
       let tokenStart = token.startIndex |> CharacterIndex.toInt;
       let tokenEnd = token.endIndex |> CharacterIndex.toInt;
       index >= tokenStart && index < tokenEnd;
     })
  |> Utility.OptionEx.of_list;
};
