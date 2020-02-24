open EditorCoreTypes;

open Oni_Core;

open Helpers;

module Diagnostic = Feature_LanguageSupport.Diagnostic;
module Definition = Feature_LanguageSupport.Definition;

let renderLine =
    (
      ~context,
      ~buffer,
      ~leftVisibleColumn,
      ~theme: Theme.t,
      ~diagnosticsMap,
      ~selectionRanges,
      ~matchingPairs,
      ~bufferHighlights,
      item,
      _offset,
    ) => {
  let index = Index.fromZeroBased(item);
  let renderDiagnostics = (d: Diagnostic.t) =>
    Draw.underline(
      ~context,
      ~buffer,
      ~leftVisibleColumn,
      ~color=Revery.Colors.red,
      d.range,
    );

  /* Draw error markers */
  switch (IntMap.find_opt(item, diagnosticsMap)) {
  | None => ()
  | Some(v) => List.iter(renderDiagnostics, v)
  };

  switch (Hashtbl.find_opt(selectionRanges, index)) {
  | None => ()
  | Some(v) =>
    List.iter(
      Draw.range(
        ~context,
        ~buffer,
        ~leftVisibleColumn,
        ~color=theme.editorSelectionBackground,
      ),
      v,
    )
  };

  /* Draw match highlights */
  let matchColor = theme.editorSelectionBackground;
  switch (matchingPairs) {
  | None => ()
  | Some((startPos, endPos)) =>
    Draw.range(
      ~context,
      ~buffer,
      ~leftVisibleColumn,
      ~color=matchColor,
      Range.{start: startPos, stop: startPos},
    );
    Draw.range(
      ~context,
      ~buffer,
      ~leftVisibleColumn,
      ~color=matchColor,
      Range.{start: endPos, stop: endPos},
    );
  };

  /* Draw search highlights */
  BufferHighlights.getHighlightsByLine(
    ~bufferId=Buffer.getId(buffer),
    ~line=index,
    bufferHighlights,
  )
  |> List.iter(
       Draw.range(
         ~context,
         ~buffer,
         ~leftVisibleColumn,
         ~padding=1.,
         ~color=theme.editorFindMatchBackground,
       ),
     );
};

let renderEmbellishments =
    (
      ~context,
      ~count,
      ~buffer,
      ~leftVisibleColumn,
      ~theme,
      ~diagnosticsMap,
      ~selectionRanges,
      ~matchingPairs,
      ~bufferHighlights,
    ) =>
  Draw.renderImmediate(
    ~context,
    ~count,
    renderLine(
      ~context,
      ~buffer,
      ~leftVisibleColumn,
      ~theme,
      ~diagnosticsMap,
      ~selectionRanges,
      ~matchingPairs,
      ~bufferHighlights,
    ),
  );

let renderDefinition =
    (
      ~context,
      ~leftVisibleColumn,
      ~cursorPosition: Location.t,
      ~buffer,
      ~bufferHighlights,
      ~theme,
      ~matchingPairs,
      ~bufferSyntaxHighlights,
      ~bufferWidthInCharacters,
    ) =>
  getTokenAtPosition(
    ~buffer,
    ~bufferHighlights,
    ~cursorLine=Index.toZeroBased(cursorPosition.line),
    ~theme,
    ~matchingPairs,
    ~bufferSyntaxHighlights,
    ~startIndex=leftVisibleColumn,
    ~endIndex=leftVisibleColumn + bufferWidthInCharacters,
    cursorPosition,
  )
  |> Option.iter((token: BufferViewTokenizer.t) => {
       let range =
         Range.{
           start:
             Location.{
               line: cursorPosition.line,
               column: token.startPosition,
             },
           stop:
             Location.{line: cursorPosition.line, column: token.endPosition},
         };
       Draw.underline(
         ~context,
         ~buffer,
         ~leftVisibleColumn,
         ~color=token.color,
         range,
       );
     });

let renderTokens =
    (~context, ~offsetY, ~theme, ~tokens, ~shouldRenderWhitespace) => {
  tokens
  |> WhitespaceTokenFilter.filter(shouldRenderWhitespace)
  |> List.iter(Draw.token(~context, ~offsetY, ~theme));
};

let renderText =
    (
      ~context,
      ~count,
      ~selectionRanges,
      ~buffer,
      ~bufferHighlights,
      ~cursorLine,
      ~theme,
      ~matchingPairs,
      ~bufferSyntaxHighlights,
      ~leftVisibleColumn,
      ~shouldRenderWhitespace,
      ~bufferWidthInCharacters,
    ) =>
  Draw.renderImmediate(
    ~context,
    ~count,
    (item, offsetY) => {
      let index = Index.fromZeroBased(item);
      let selectionRange =
        switch (Hashtbl.find_opt(selectionRanges, index)) {
        | None => None
        | Some(v) =>
          switch (List.length(v)) {
          | 0 => None
          | _ => Some(List.hd(v))
          }
        };
      let tokens =
        getTokensForLine(
          ~buffer,
          ~bufferHighlights,
          ~cursorLine,
          ~theme,
          ~matchingPairs,
          ~bufferSyntaxHighlights,
          ~selection=selectionRange,
          leftVisibleColumn,
          leftVisibleColumn + bufferWidthInCharacters,
          item,
        );

      renderTokens(
        ~context,
        ~offsetY,
        ~theme,
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
      ~leftVisibleColumn,
      ~theme,
      ~diagnosticsMap,
      ~selectionRanges,
      ~matchingPairs,
      ~bufferHighlights,
      ~cursorPosition: Location.t,
      ~definition,
      ~bufferSyntaxHighlights,
      ~shouldRenderWhitespace,
      ~bufferWidthInCharacters,
    ) => {
  renderEmbellishments(
    ~context,
    ~count,
    ~buffer,
    ~leftVisibleColumn,
    ~theme,
    ~diagnosticsMap,
    ~selectionRanges,
    ~matchingPairs,
    ~bufferHighlights,
  );

  if (Definition.isAvailable(
        Buffer.getId(buffer),
        cursorPosition,
        definition,
      )) {
    renderDefinition(
      ~context,
      ~leftVisibleColumn,
      ~cursorPosition,
      ~buffer,
      ~bufferHighlights,
      ~theme,
      ~matchingPairs,
      ~bufferSyntaxHighlights,
      ~bufferWidthInCharacters,
    );
  };

  renderText(
    ~context,
    ~count,
    ~selectionRanges,
    ~buffer,
    ~bufferHighlights,
    ~cursorLine=Index.toZeroBased(cursorPosition.line),
    ~theme,
    ~matchingPairs,
    ~bufferSyntaxHighlights,
    ~leftVisibleColumn,
    ~shouldRenderWhitespace,
    ~bufferWidthInCharacters,
  );
};
