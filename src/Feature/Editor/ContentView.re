open EditorCoreTypes;
open Revery;

open Oni_Core;

open Helpers;

module Diagnostic = Feature_LanguageSupport.Diagnostic;
module Definition = Feature_LanguageSupport.Definition;

let renderLine =
    (
      ~transform,
      ~buffer,
      ~leftVisibleColumn,
      ~scrollY,
      ~editorFont,
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
      ~buffer,
      ~leftVisibleColumn,
      ~transform,
      ~scrollY,
      ~editorFont,
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
        ~buffer,
        ~leftVisibleColumn,
        ~transform,
        ~scrollY,
        ~editorFont,
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
      ~buffer,
      ~leftVisibleColumn,
      ~transform,
      ~scrollY,
      ~editorFont,
      ~color=matchColor,
      Range.{start: startPos, stop: startPos},
    );
    DrawPrimitives.renderRange(
      ~buffer,
      ~leftVisibleColumn,
      ~transform,
      ~scrollY,
      ~editorFont,
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
         ~buffer,
         ~editorFont,
         ~leftVisibleColumn,
         ~transform,
         ~scrollY,
         ~offset=2.0,
         ~color=theme.editorFindMatchBackground,
       ),
     );
};

let renderEmbellishments =
    (
      ~scrollY,
      ~rowHeight,
      ~height,
      ~count,
      ~transform,
      ~buffer,
      ~editorFont,
      ~leftVisibleColumn,
      ~theme,
      ~diagnosticsMap,
      ~selectionRanges,
      ~matchingPairs,
      ~bufferHighlights,
    ) =>
  ImmediateList.render(
    ~scrollY,
    ~rowHeight,
    ~height=float(height),
    ~count,
    ~render=
      renderLine(
        ~transform,
        ~buffer,
        ~scrollY,
        ~editorFont,
        ~leftVisibleColumn,
        ~theme,
        ~diagnosticsMap,
        ~selectionRanges,
        ~matchingPairs,
        ~bufferHighlights,
      ),
    (),
  );

let renderDefinition =
    (
      ~leftVisibleColumn,
      ~layout: EditorLayout.t,
      ~cursorPosition: Location.t,
      ~buffer,
      ~transform,
      ~scrollY,
      ~editorFont,
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
         ~buffer,
         ~editorFont,
         ~leftVisibleColumn,
         ~transform,
         ~scrollY,
         ~color=token.color,
         range,
       );
     });

let renderTokens =
    (
      ~editorFont,
      ~theme,
      ~tokens,
      ~scrollX,
      ~scrollY,
      ~transform,
      ~shouldRenderWhitespace,
    ) => {
  tokens
  |> WhitespaceTokenFilter.filter(shouldRenderWhitespace)
  |> List.iter(
       DrawPrimitives.renderToken(
         ~editorFont,
         ~theme,
         ~scrollX,
         ~scrollY,
         ~transform,
       ),
     );
};

let renderText =
    (
      ~scrollY,
      ~rowHeight,
      ~height,
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
      ~editorFont,
      ~editor: Editor.t,
      ~transform,
      ~shouldRenderWhitespace,
    ) =>
  ImmediateList.render(
    ~scrollY,
    ~rowHeight,
    ~height=float(height),
    ~count,
    ~render=
      (item, offset) => {
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

        let _ =
          renderTokens(
            ~editorFont,
            ~theme,
            ~tokens,
            ~scrollX=editor.scrollX,
            ~scrollY=offset,
            ~transform,
            ~shouldRenderWhitespace,
          );
        ();
      },
    (),
  );

let render =
    (
      ~scrollY,
      ~rowHeight,
      ~height,
      ~count,
      ~transform,
      ~buffer,
      ~editor,
      ~editorFont,
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
    ~scrollY,
    ~rowHeight,
    ~height,
    ~count,
    ~transform,
    ~buffer,
    ~editorFont,
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
      ~leftVisibleColumn,
      ~layout,
      ~cursorPosition,
      ~buffer,
      ~editorFont,
      ~transform,
      ~scrollY,
      ~bufferHighlights,
      ~theme,
      ~matchingPairs,
      ~bufferSyntaxHighlights,
    );
  };

  renderText(
    ~scrollY,
    ~rowHeight,
    ~height,
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
    ~editor,
    ~editorFont,
    ~transform,
    ~shouldRenderWhitespace,
  );
};
