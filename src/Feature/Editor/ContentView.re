open EditorCoreTypes;

open Oni_Core;

open Helpers;

module Diagnostic = Feature_LanguageSupport.Diagnostic;

let renderLine =
    (
      ~context,
      ~buffer,
      ~colors: Colors.t,
      ~diagnosticsMap,
      ~selectionRanges,
      ~matchingPairs: option((CharacterPosition.t, CharacterPosition.t)),
      ~bufferHighlights,
      ~languageSupport,
      item,
      _offset,
    ) => {
  let index = EditorCoreTypes.LineNumber.ofZeroBased(item);
  let renderDiagnostics = (colors: Colors.t, diagnostic: Diagnostic.t) =>
    Draw.underline(~context, ~color=colors.errorForeground, diagnostic.range);

  /* Draw error markers */
  switch (IntMap.find_opt(item, diagnosticsMap)) {
  | None => ()
  | Some(diagnostics) => List.iter(renderDiagnostics(colors), diagnostics)
  };

  switch (Hashtbl.find_opt(selectionRanges, index)) {
  | None => ()
  | Some(selections) =>
    List.iter(
      Draw.rangeByte(~context, ~color=colors.selectionBackground),
      selections,
    )
  };

  /* Draw match highlights */
  switch (matchingPairs) {
  | None => ()
  | Some((startPos, endPos)) =>
    Draw.rangeCharacter(
      ~context,
      ~color=colors.selectionBackground,
      CharacterRange.{start: startPos, stop: startPos},
    );
    Draw.rangeCharacter(
      ~context,
      ~color=colors.selectionBackground,
      CharacterRange.{start: endPos, stop: endPos},
    );
  };

  let bufferId = Buffer.getId(buffer);
  /* Draw search highlights */
  BufferHighlights.getHighlightsByLine(
    ~bufferId,
    ~line=index,
    bufferHighlights,
  )
  |> List.iter(
       Draw.rangeByte(
         ~context,
         ~padding=1.,
         ~color=colors.findMatchBackground,
       ),
     );

  /* Draw document highlights */
  Feature_LanguageSupport.DocumentHighlights.getByLine(
    ~bufferId,
    ~line=index |> EditorCoreTypes.LineNumber.toZeroBased,
    languageSupport,
  )
  |> List.iter(
       Draw.rangeCharacter(
         ~context,
         ~padding=1.,
         ~color=colors.findMatchBackground,
       ),
     );
};

let renderEmbellishments =
    (
      ~context,
      ~count,
      ~buffer,
      ~colors,
      ~diagnosticsMap,
      ~selectionRanges,
      ~matchingPairs,
      ~bufferHighlights,
      ~languageSupport,
    ) =>
  Draw.renderImmediate(
    ~context,
    ~count,
    renderLine(
      ~context,
      ~buffer,
      ~colors,
      ~diagnosticsMap,
      ~selectionRanges,
      ~matchingPairs,
      ~bufferHighlights,
      ~languageSupport,
    ),
  );

let renderDefinition =
    (
      ~context,
      ~bufferId,
      ~languageSupport,
      ~leftVisibleColumn,
      ~cursorPosition: CharacterPosition.t,
      ~editor,
      ~bufferHighlights,
      ~colors,
      ~matchingPairs: option((CharacterPosition.t, CharacterPosition.t)),
      ~bufferSyntaxHighlights,
      ~bufferWidthInCharacters,
    ) =>
  getTokenAtPosition(
    ~editor,
    ~bufferHighlights,
    ~cursorLine=EditorCoreTypes.LineNumber.toZeroBased(cursorPosition.line),
    ~colors,
    ~matchingPairs,
    ~bufferSyntaxHighlights,
    ~startIndex=leftVisibleColumn,
    ~endIndex=leftVisibleColumn + bufferWidthInCharacters,
    cursorPosition,
  )
  |> Option.iter((token: BufferViewTokenizer.t) => {
       let range =
         CharacterRange.{
           start:
             CharacterPosition.{
               line: cursorPosition.line,
               character: token.startIndex,
             },
           stop:
             CharacterPosition.{
               line: cursorPosition.line,
               character: token.endIndex,
             },
         };

       // Double-check that the range of the token falls into our definition position

       Feature_LanguageSupport.Definition.getAt(
         ~bufferId,
         ~range,
         languageSupport,
       )
       |> Option.iter(_ => {
            Draw.underline(~context, ~color=token.color, range)
          });
     });

let renderTokens =
    (~context, ~line, ~colors, ~tokens, ~shouldRenderWhitespace) => {
  tokens
  |> WhitespaceTokenFilter.filter(shouldRenderWhitespace)
  |> List.iter(Draw.token(~context, ~line, ~colors));
};

let renderText =
    (
      ~context,
      ~count,
      ~selectionRanges,
      ~editor,
      ~bufferHighlights,
      ~cursorLine,
      ~colors,
      ~matchingPairs,
      ~bufferSyntaxHighlights,
      ~shouldRenderWhitespace,
      ~bufferWidthInPixels,
    ) =>
  Draw.renderImmediate(
    ~context,
    ~count,
    (item, _offsetY) => {
      let index = EditorCoreTypes.LineNumber.ofZeroBased(item);
      let selectionRange =
        switch (Hashtbl.find_opt(selectionRanges, index)) {
        | None => None
        | Some(v) =>
          switch (List.length(v)) {
          | 0 => None
          | _ => Some(List.hd(v))
          }
        };
      let bufferLine = Editor.viewLine(editor, item).contents;
      let startPixel = Editor.scrollX(editor);
      let startCharacter =
        BufferLine.Slow.getIndexFromPixel(~pixel=startPixel, bufferLine)
        |> CharacterIndex.toInt;
      let endCharacter =
        BufferLine.Slow.getIndexFromPixel(
          ~pixel=startPixel +. float(bufferWidthInPixels),
          bufferLine,
        )
        |> CharacterIndex.toInt;

      let tokens =
        getTokensForLine(
          ~editor,
          ~bufferHighlights,
          ~cursorLine,
          ~colors,
          ~matchingPairs,
          ~bufferSyntaxHighlights,
          ~selection=selectionRange,
          startCharacter,
          endCharacter + 1,
          item,
        );

      renderTokens(
        ~context,
        ~line=item |> EditorCoreTypes.LineNumber.ofZeroBased,
        ~colors,
        ~tokens,
        ~shouldRenderWhitespace,
      );
    },
  );

let render =
    (
      ~context,
      ~count,
      ~buffer,
      ~editor,
      ~leftVisibleColumn,
      ~colors,
      ~diagnosticsMap,
      ~selectionRanges,
      ~matchingPairs: option((CharacterPosition.t, CharacterPosition.t)),
      ~bufferHighlights,
      ~cursorPosition: CharacterPosition.t,
      ~languageSupport,
      ~bufferSyntaxHighlights,
      ~shouldRenderWhitespace,
      ~bufferWidthInCharacters,
      ~bufferWidthInPixels,
    ) => {
  renderEmbellishments(
    ~context,
    ~count,
    ~buffer,
    ~colors,
    ~diagnosticsMap,
    ~selectionRanges,
    ~matchingPairs,
    ~bufferHighlights,
    ~languageSupport,
  );

  let bufferId = Buffer.getId(buffer);
  if (Feature_LanguageSupport.Definition.isAvailable(
        ~bufferId,
        languageSupport,
      )) {
    renderDefinition(
      ~bufferId,
      ~languageSupport,
      ~context,
      ~editor,
      ~leftVisibleColumn,
      ~cursorPosition,
      ~bufferHighlights,
      ~colors,
      ~matchingPairs,
      ~bufferSyntaxHighlights,
      ~bufferWidthInCharacters,
    );
  };

  renderText(
    ~context,
    ~count,
    ~selectionRanges,
    ~editor,
    ~bufferHighlights,
    ~cursorLine=EditorCoreTypes.LineNumber.toZeroBased(cursorPosition.line),
    ~colors,
    ~matchingPairs,
    ~bufferSyntaxHighlights,
    ~shouldRenderWhitespace,
    ~bufferWidthInPixels,
  );
};
