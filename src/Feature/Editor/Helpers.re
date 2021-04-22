open EditorCoreTypes;

let getTokensForLine =
    (
      ~editor,
      ~vim,
      ~cursorLine,
      ~colors: Colors.t,
      ~matchingPairs,
      ~bufferSyntaxHighlights,
      ~ignoreMatchingPairs=false,
      ~selection=None,
      ~scrollX,
      i,
    ) =>
  if (i >= Editor.totalViewLines(editor)) {
    [];
  } else {
    let bufferId = Editor.getBufferId(editor);

    let isActiveLine = i == cursorLine;
    let defaultBackground =
      isActiveLine ? colors.lineHighlightBackground : colors.editorBackground;

    let bufferLine = Editor.viewLineToBufferLine(i, editor);
    let tokenColors =
      Feature_Syntax.getTokens(
        ~bufferId,
        ~line=bufferLine,
        bufferSyntaxHighlights,
      );

    let idx = EditorCoreTypes.LineNumber.ofZeroBased(i);
    let searchHighlights =
      Feature_Vim.getSearchHighlightsByLine(~bufferId, ~line=idx, vim);

    let matchingPairIndex =
      switch (matchingPairs) {
      | None => None
      | Some((startPos: CharacterPosition.t, endPos: CharacterPosition.t))
          when !ignoreMatchingPairs =>
        let maybeStartPosByte = Editor.characterToByte(startPos, editor);
        let maybeEndPosByte = Editor.characterToByte(endPos, editor);
        if (EditorCoreTypes.LineNumber.toZeroBased(startPos.line) == i) {
          maybeStartPosByte |> Option.map(BytePosition.byte);
        } else if (EditorCoreTypes.LineNumber.toZeroBased(endPos.line) == i) {
          maybeEndPosByte |> Option.map(BytePosition.byte);
        } else {
          None;
        };
      | _ => None
      };

    let colorizer =
      BufferLineColorizer.create(
        ~defaultBackgroundColor=defaultBackground,
        ~defaultForegroundColor=colors.editorForeground,
        ~selectionHighlights=selection,
        ~selectionColor=colors.selectionBackground,
        ~matchingPair=matchingPairIndex,
        ~searchHighlights,
        ~searchHighlightColor=colors.findMatchBackground,
        tokenColors,
      );

    Editor.viewTokens(~colorizer, ~scrollX, ~line=i, editor);
  };
