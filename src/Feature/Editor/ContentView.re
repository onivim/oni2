open EditorCoreTypes;
open Revery;

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
    DrawPrimitives.drawUnderline(
      ~context,
      ~buffer,
      ~leftVisibleColumn,
      ~color=Colors.red,
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
      DrawPrimitives.renderRange(
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
    DrawPrimitives.renderRange(
      ~context,
      ~buffer,
      ~leftVisibleColumn,
      ~color=matchColor,
      Range.{start: startPos, stop: startPos},
    );
    DrawPrimitives.renderRange(
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
       DrawPrimitives.renderRange(
         ~context,
         ~buffer,
         ~leftVisibleColumn,
         ~offset=2.0,
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
  DrawPrimitives.renderImmediate(
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
      ~layout: EditorLayout.t,
      ~cursorPosition: Location.t,
      ~buffer,
      ~bufferHighlights,
      ~theme,
      ~matchingPairs,
      ~bufferSyntaxHighlights,
    ) =>
  getTokenAtPosition(
    ~buffer,
    ~bufferHighlights,
    ~cursorLine=Index.toZeroBased(cursorPosition.line),
    ~theme,
    ~matchingPairs,
    ~bufferSyntaxHighlights,
    ~startIndex=leftVisibleColumn,
    ~endIndex=leftVisibleColumn + layout.bufferWidthInCharacters,
    cursorPosition,
  )
  |> Utility.Option.iter((token: BufferViewTokenizer.t) => {
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
       DrawPrimitives.drawUnderline(
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
  |> List.iter(DrawPrimitives.renderToken(~context, ~offsetY, ~theme));
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
      ~layout: EditorLayout.t,
      ~shouldRenderWhitespace,
    ) =>
  DrawPrimitives.renderImmediate(
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
          leftVisibleColumn + layout.bufferWidthInCharacters,
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
      ~layout,
      ~bufferSyntaxHighlights,
      ~shouldRenderWhitespace,
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
      ~layout,
      ~cursorPosition,
      ~buffer,
      ~bufferHighlights,
      ~theme,
      ~matchingPairs,
      ~bufferSyntaxHighlights,
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
    ~layout,
    ~shouldRenderWhitespace,
  );
};
