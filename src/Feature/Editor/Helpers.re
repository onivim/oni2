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
    let {tokens, }: Editor.viewTokens = Editor.viewTokens(~line=1, ~position=0, editor);
    tokens;
//    let line = viewLine.contents;
//    let length = BufferLine.lengthInBytes(line);
//    let startIndex = max(0, startIndex);
//    let endIndex = max(0, endIndex);
//    let bufferId = Editor.getBufferId(editor);
//
//    if (length == 0) {
//      [];
//    } else {
//      let idx = Index.fromZeroBased(i);
//      let highlights =
//        BufferHighlights.getHighlightsByLine(
//          ~bufferId,
//          ~line=idx,
//          bufferHighlights,
//        );
//
//      let isActiveLine = i == cursorLine;
//      let defaultBackground =
//        isActiveLine
//          ? colors.lineHighlightBackground : colors.editorBackground;
//
//      let matchingPairIndex =
//        switch (matchingPairs) {
//        | None => None
//        | Some((startPos: Location.t, endPos: Location.t))
//            when !ignoreMatchingPairs =>
//          if (Index.toZeroBased(startPos.line) == i) {
//            Some(Index.toZeroBased(startPos.column));
//          } else if (Index.toZeroBased(endPos.line) == i) {
//            Some(Index.toZeroBased(endPos.column));
//          } else {
//            None;
//          }
//        | _ => None
//        };
//
//      let tokenColors =
//        Feature_Syntax.getTokens(
//          ~bufferId,
//          ~line=Index.fromZeroBased(i),
//          bufferSyntaxHighlights,
//        );
//
//      let startByte = BufferLine.getByteFromIndex(~index=startIndex, line);
//
//      let colorizer =
//        BufferLineColorizer.create(
//          ~startByte,
//          ~defaultBackgroundColor=defaultBackground,
//          ~defaultForegroundColor=colors.editorForeground,
//          ~selectionHighlights=selection,
//          ~selectionColor=colors.selectionBackground,
//          ~matchingPair=matchingPairIndex,
//          ~searchHighlights=highlights,
//          ~searchHighlightColor=colors.findMatchBackground,
//          tokenColors,
//        );
//
//      BufferViewTokenizer.tokenize(~startIndex, ~endIndex, line, colorizer);
//    };
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
       let tokenStart = token.startIndex |> Index.toZeroBased;
       let tokenEnd = token.endIndex |> Index.toZeroBased;
       index >= tokenStart && index < tokenEnd;
     })
  |> Utility.OptionEx.of_list;
};
